#include "button_key.h"
#include "74HC595.h"
#include "seg_led.h"
#include "event.h"
#include "pwm.h"

#define KEY_IN_DET_PORT GPIOA
#define KEY_IN_DET_PIN  GPIO_Pin_11

#define KEY_NUM 5
#define BUTTON_SHORT_TIME  2 //100ms
#define BUTTON_LONG_TIME  60  //3s

struct node_struct_t Button_Key_node;
uint8_t ButtonScanShift[KEY_NUM] = {KEY_CANCEL_SHORT, KEY_DEL_SHORT, KEY_OK_SHORT, KEY_INIT_SHORT, KEY_ADD_SHORT};

void Button_Key_Scan(void *priv);

void Button_Key_Init(void)  //TODO 以后要改成中断方式
{
		GPIO_InitTypeDef GPIO_InitStruct;
	
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = KEY_IN_DET_PIN;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		        
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(KEY_IN_DET_PORT, &GPIO_InitStruct);
	HC595_Updata(SER_DOT_INTERFACE, 00);
	lklt_insert(&Button_Key_node, Button_Key_Scan, NULL, 50/2);
}


void Button_Key_Scan(void *priv)
{
		static uint16_t KeyDebounceTime[KEY_NUM], i;
		uint8_t KeyValue = 0;
	
		if(GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)==0)
		{
			for(i=0; i<=KEY_NUM; i++)
			{
				HC595_Updata(SER_DOT_INTERFACE, ButtonScanShift[i]);
				if(GPIO_ReadInputDataBit( KEY_IN_DET_PORT, KEY_IN_DET_PIN)==1)
					KeyDebounceTime[i]++;
				else
					KeyDebounceTime[i]=0;
			}
			HC595_Updata(SER_DOT_INTERFACE, 0x00);		
		}	
		else
		{
			for(i=0; i<KEY_NUM; i++)
			{
				if(KeyDebounceTime[i]>=BUTTON_LONG_TIME)
					KeyValue |= (ButtonScanShift[i] | 0x80);
				else if((KeyDebounceTime[i]>=BUTTON_SHORT_TIME)&&(KeyDebounceTime[i]<BUTTON_LONG_TIME))
					KeyValue |= ButtonScanShift[i];
				
				KeyDebounceTime[i] = 0;
			}
		}
		if(KeyValue!=0)
		{
			Hal_EventTypedef evt;
			
			evt.event = BUTTON_KEY_EVENT;
			evt.data.KeyValude = KeyValue;
			USBH_PutEvent(evt);
			Hal_Beep_Blink (1, 80, 30);
			printf("KeyValue: %X\r\n", KeyValue);
		}
		
}







