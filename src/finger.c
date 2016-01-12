#include "uart.h"
#include "Link_list.h"
#include "event.h"
#include "finger.h"

struct node_struct_t finger_uart_scan_node;
static void Finger_Scan(void);


void finger_init(void)
{
	uart1_Init();
	lklt_insert(&finger_uart_scan_node, Finger_Scan, NULL, 50*TRAV_INTERVAL);
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

void Finger_Regist(void)//两次按压指纹注册
{
	UsartClrBuf();
	UsartSend("\xf5\x02\x00\x00\x00\x00\x02\xf5", 8, 0);
}

uint16_t Get_Finger_Num(uint16_t *num)
{
	uint8_t ack[8];
	uint16_t len;
	
	UsartClrBuf();
	Api_Send_Data("\xf5\x09\x00\x00\x00\x00\x09\xf5", 8, 1);
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


void Match_finger(void)
{
	UsartClrBuf();
    UsartSend("\xf5\x0C\x00\x00\x00\x00\x0C\xf5", 8, 0);
}














