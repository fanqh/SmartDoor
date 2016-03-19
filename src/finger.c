#include "uart.h"
#include "Link_list.h"
#include "event.h"
#include "finger.h"
#include <string.h>
#include "delay.h"
#include "led_blink.h"

#define FINGER_RF_LDO_PIN			GPIO_Pin_6
#define FINGER_WAKEUP_DETECT_PIN	GPIO_Pin_12

struct node_struct_t finger_uart_scan_node;
uint8_t finger_cmd[8]={0xF5,0X00,0X00,0X00,0X00,0X00,};
finger_state_t finger_state;
uint8_t is_finger_ok = 0; 
extern uint8_t is_Err_Warm_Flag;


void Finger_LDO_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = FINGER_RF_LDO_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF,ENABLE);
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}

void finger_wakeup_detect_pin_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = FINGER_WAKEUP_DETECT_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t is_finger_wakeup(void)
{
	if( GPIO_ReadInputDataBit(GPIOA, FINGER_WAKEUP_DETECT_PIN)==Bit_SET)
		return 1;
	else 
		return 0;
	//return GPIO_ReadInputDataBit(GPIOA, FINGER_WAKEUP_DETECT_PIN);
}

void Finger_RF_LDO_Enable(void)
{
	GPIO_SetBits(GPIOF, FINGER_RF_LDO_PIN);
}

void Finger_RF_LDO_Disable(void)
{
	GPIO_ResetBits(GPIOF, FINGER_RF_LDO_PIN);
}

static void Finger_Sent_Byte8_Cmd(uint8_t *buff, uint8_t block)
{
	uint8_t cmd[8], i, crc;
	memcpy(cmd,buff,8);
	
	crc = 0;
	for(i=1;i<6;i++)
		crc ^= cmd[i];
	cmd[6] = crc;
	UsartClrBuf();
	UsartSend(cmd, 8, block);		
}

void finger_init(void)
{
	uint8_t reset_count = 0;
	
	uart2_Init();
	finger_state = FP_IDLY;
	delay_ms(150);
	finger_wakeup_detect_pin_init();
//	lklt_insert(&finger_uart_scan_node, Finger_Scan, NULL, 1*TRAV_INTERVAL);
	while(reset_count<3)
	{
		if(Finger_Set_DenyingSame()==ACK_SUCCESS)
			break;
		else
		{
			reset_count++;
			Finger_RF_LDO_Disable();
			delay_ms(10);
			Finger_RF_LDO_Enable();
			delay_ms(150);
			printf("reset %d\r\n", reset_count);
		}	
	}
	if(reset_count<3)
		is_finger_ok=1;
	
}

void Finger_Scan(void)
{
	Hal_EventTypedef evt;
	uint16_t  len,i;
	uint8_t ack[32];
	
	memset(ack, 0, sizeof(ack));
	len = GetUartSize();
	if((uart_block_flag!=1)&&(len>=8))
	{
		evt.event = FINGER_EVENT;
		UsartGetBlock(ack, 8, 1);
		for(i=0;i<8;i++)
			printf("%2X ",ack[i]);
		if(finger_state==FP_REGISTING)
		{
			LED_BB_OFF();
			finger_state = FP_IDLY;
			if(ack[1]==REGIST1_CMD)
			{
				evt.data.Buff[0] = REGIST1_CMD;
				evt.data.Buff[1] = ack[4]; ///19/9 原来为ack[3]
				PutEvent(evt);
			}
			else if(ack[1]==REGIST2_CMD)
			{
				if((ack[4]==ACK_FAIL) || (ack[4]==ACK_FULL) || (ack[4]==ACK_IMAGEFAIL) || (ack[4]==ACK_USER_EXIST))
				{	
					evt.data.Buff[0] = REGIST2_CMD;
					evt.data.Buff[1] = ack[4]; 
					PutEvent(evt);
					
				}
				else if(ack[4]==ACK_SUCCESS)
				{
					evt.data.Buff[0] = REGIST2_CMD;   //成功和对比失败
					evt.data.Buff[1] = ACK_SUCCESS; 
					evt.data.Buff[2] = ack[3]; 
					evt.data.Buff[3] = ack[2]; 
					PutEvent(evt);
					printf("reg2 sucess\r\n");
				}
				else if(ack[4]==ACK_TIMEOUT)
					Finger_Regist_CMD2();
				
			}
			if(ack[1]==REGIST3_CMD)
			{
				if((ack[4]==ACK_FAIL) || (ack[4]==ACK_FULL) || (ack[4]==ACK_IMAGEFAIL) || (ack[4]==ACK_USER_EXIST))
				{	
					evt.data.Buff[0] = REGIST3_CMD;
					evt.data.Buff[1] = ack[4]; 
					PutEvent(evt);
				}
				else if(ack[4]==ACK_SUCCESS)
				{
					evt.data.Buff[0] = REGIST3_CMD;   
					evt.data.Buff[1] = ACK_SUCCESS; 
					evt.data.Buff[2] = ack[3]; 
					evt.data.Buff[3] = ack[2]; 
					PutEvent(evt);
				}
				else if(ack[4]==ACK_TIMEOUT)
					Finger_Regist_CMD2();
//				else //if(ack[4]==ACK_BREAK)
//					finger_state = FP_IDLY;
			}
		}
		else if(finger_state==FP_1_N_MATCH)
		{
			finger_state = FP_IDLY;
			if(ack[1]==MATCH_CMD)
			{
				if(is_Err_Warm_Flag==1)
					return;
				if(ack[4]==ACK_TIMEOUT )//|| ack[4]==ACK_FAIL
				{
					Match_finger();
				}
				else //if(ack[4]==ACK_SUCCESS)
				{
					evt.data.Buff[0] = MATCH_CMD;
					evt.data.Buff[1] = ack[4];
					evt.data.Buff[2] = ack[3];
					evt.data.Buff[3] = ack[2];
						
					PutEvent(evt);
//					finger_state = FP_IDLY;
				}
//				else
//					finger_state = FP_IDLY;
			}
		}
		else
			finger_state = FP_IDLY;
	UsartClrBuf();
	}	
}
void Exit_Finger_Current_Operate(void)  //随便一个即可返回的指令即可
{
	if(is_finger_ok) 
	{
		uint8_t ack[8];
		uint8_t s[8]={0xf5,0x09,00,00,00,00,0x09,0xf5};
		
		LED_BB_OFF();
		Finger_Sent_Byte8_Cmd(s, 1);
		UsartGetBlock(ack, 8, 100);
		UsartClrBuf();
		finger_state = FP_IDLY;
		
	}

}
void Finger_Regist_CMD1(void)
{
	uint8_t s[8]={0xf5,0x01,0x00,00,01,00,0x00,0xf5};

	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	UsartClrBuf();
	finger_state = FP_REGISTING;
	Finger_Sent_Byte8_Cmd(s, 0);
}

void Finger_Regist_CMD2(void)
{
	uint8_t s[8]={0xf5,0x02,0x00,00,01,00,0x01,0xf5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	UsartClrBuf();
	finger_state = FP_REGISTING;
	Finger_Sent_Byte8_Cmd(s, 0);
	LED_BB_ON();
}
void Finger_Regist_CMD3(void)
{
	uint8_t s[8]={0xf5,0x03,0x00,00,01,00,0x01,0xf5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	UsartClrBuf();
	finger_state = FP_REGISTING;
	Finger_Sent_Byte8_Cmd(s, 0);
	LED_BB_ON();
}
uint8_t Finger_Set_DenyingSame(void)
{
	uint8_t ack[8];
	uint8_t s[8]={0xf5,0x2d,0x00,0x01,00,00,0x2c,0xf5};
	uint16_t len;
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	memset(ack,0,8);
	UsartClrBuf();
	Finger_Sent_Byte8_Cmd(s, 1);
	len = UsartGetBlock(ack, 8, 100);
	if((len>=8) && (ack[1]==0x2d) && (ack[4]==ACK_SUCCESS))
		return ACK_SUCCESS;
	else
		return ACK_FAIL;
}


uint16_t Get_Finger_Num(uint16_t *num)
{
	uint8_t ack[8];
	uint16_t len;
	uint8_t s[8]={0xf5,0x09,00,00,00,00,0x09,0xf5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	UsartClrBuf();
	memset(ack, 0, sizeof(ack));
	Finger_Sent_Byte8_Cmd(s, 1);
	
	len = UsartGetBlock(ack, 8, 1000);
	
    if(len>=8 && ack[4]==0x00)
    {
        *num = ack[2]*256+ack[3];
        return ACK_SUCCESS;
    }
    else
    {
       return ACK_FAIL;
    }  
	
}


//1： 删除成功
//0:  删除失败
uint8_t Delete_All_Finger(void)
{
	uint8_t ack[8], len;
	uint8_t s[8] = {0xF5,0X05,0X00,0X00,0X00,0X00,0x00,0xF5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	
	memset(ack, 0, sizeof(ack));
	Finger_Sent_Byte8_Cmd(s, 1);	
	
	len = UsartGetBlock(ack, 8, 1000);
	
	if(len>=8 && ack[4]==ACK_SUCCESS)
        return 1;
    else
		return 0;
}

//1： 删除成功
//0:  删除失败
uint8_t Delelte_ONE_Finger(uint16_t id)
{
	uint8_t ack[8], len;
	uint8_t s[8] = {0xF5,0X04,0X00,0X00,0X00,0X00,0x00,0xF5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	
	s[2] = (uint8_t)(id>>8)&0xff;
	s[3] = (uint8_t)(id&0xff);
	memset(ack, 0, sizeof(ack));
	Finger_Sent_Byte8_Cmd(s, 1);	
	
	len = UsartGetBlock(ack, 8, 1000);
	
	if(len>=8 && ((ack[4]==ACK_SUCCESS)||(ack[4]==ACK_NOUSER)))
        return 1;
    else
		return 0;
}


void Match_finger(void)
{
	uint8_t s[8]={0xf5,0x0C,0x00,00,00,00,0x0C,0xf5};
	
	if(finger_state != FP_IDLY)
	{
		Exit_Finger_Current_Operate();
	}
	
	UsartClrBuf();
	finger_state = FP_1_N_MATCH;
    Finger_Sent_Byte8_Cmd(s, 0);
}














