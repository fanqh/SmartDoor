#include "uart.h"
#include "Link_list.h"
#include "event.h"
#include "finger.h"
#include <string.h>

struct node_struct_t finger_uart_scan_node;
static void Finger_Scan(void);
uint8_t finger_cmd[8]={0xF5,0X00,0X00,0X00,0X00,0X00,};


void finger_init(void)
{
	uart1_Init();
	lklt_insert(&finger_uart_scan_node, Finger_Scan, NULL, 50*TRAV_INTERVAL);
}

void Finger_Sent_Byte8_Cmd(uint8_t *buff, uint8_t block)
{
	uint8_t cmd[8], i, crc;
	memcpy(cmd,buff,8);
	for(i=1;i<6;i++)
		crc ^= buff[i];
	UsartClrBuf();
	UsartSend(cmd, 8, block);		
}


static void Finger_Scan(void)
{
	Hal_EventTypedef evt;
	
	if((uart_block_flag!=1)&&(GetUartSize()>=8))
	{
		evt.event = FINGER_EVENT;
		USBH_PutEvent(evt);
	}	
}

void Finger_Regist_CMD2(void)//两次按压指纹注册
{
	UsartClrBuf();
	UsartSend("\xf5\x02\x00\x00\x00\x00\x02\xf5", 8, 0);
}
void Finger_Regist_CMD3(void)
{
	UsartClrBuf();
	UsartSend("\xf5\x03\x00\x00\x01\x00\x02\xf5", 8, 0);	
}


uint16_t Get_Finger_Num(uint16_t *num)
{
	uint8_t ack[8];
	uint16_t len;
	
	UsartClrBuf();
	UsartSend("\xf5\x09\x00\x00\x00\x00\x09\xf5", 8, 1);
	memset(ack, 0, sizeof(ack));
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

void Exit_Finger_Current_Operate(void)  //随便一个即可返回的指令即可
{
#ifdef FINGER
	uint8_t ack[8];
	
	UsartSend("\xf5\x09\x00\x00\x00\x00\x09\xf5", 8, 1);
	UsartGetBlock(ack, 8, 100);
	UsartClrBuf();
	
#endif

}
//1： 删除成功
//0:  删除失败
uint8_t Delete_All_Finger(void)
{
	uint8_t ack[8], len;
	uint8_t s[8] = {0xF5,0X05,0X00,0X00,0X00,0X00,0x00,0xF5};
	
	Finger_Sent_Byte8_Cmd(s, 1);	
	memset(ack, 0, sizeof(ack));
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
	
	s[2] = (uint8_t)(id>>8)&0xff;
	s[3] = (uint8_t)(id&0xff);
	Finger_Sent_Byte8_Cmd(s, 1);	
	memset(ack, 0, sizeof(ack));
	len = UsartGetBlock(ack, 8, 1000);
	
	if(len>=8 && ack[4]==ACK_SUCCESS)
        return 1;
    else
		return 0;
}


void Match_finger(void)
{
	UsartClrBuf();
    UsartSend("\xf5\x0C\x00\x00\x00\x00\x0C\xf5", 8, 0);
}














