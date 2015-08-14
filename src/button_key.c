#include "button_key.h"
#include "74HC595.h"
#include "seg_led.h"
#include "event.h"
#include "pwm.h"


#define KEY_NUM 5
#define BUTTON_SHORT_TIME  2 //20ms
#define BUTTON_LONG_TIME  100  //600ms

struct node_struct_t Button_Key_node;
uint8_t ButtonScanShift[KEY_NUM] = {KEY_CANCEL_SHORT, KEY_DEL_SHORT, KEY_OK_SHORT, KEY_INIT_SHORT, KEY_ADD_SHORT};

void Button_Key_Scan(void *priv);


void Button_KeyInDec_Gpio_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin = KEY_IN_DET_PIN;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		        
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(KEY_IN_DET_PORT, &GPIO_InitStruct);
}
void Button_Key_Init(void)  //TODO 以后要改成中断方式
{
	HC595_Updata(SER_DOT_INTERFACE, 00);
	lklt_insert(&Button_Key_node, Button_Key_Scan, NULL, 1*TRAV_INTERVAL);//2ms
}
uint8_t Get_Key_In0_Status(void)
{
		return GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN);
}


void Button_Key_Scan(void *priv)
{
		static uint16_t KeyDebounceTime[KEY_NUM], i;
		uint8_t KeyValue = 0;
	
		if(GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)==0)//have key in state of hold on
		{
			for(i=0; i<=KEY_NUM; i++)
			{
				HC595_Updata(SER_DOT_INTERFACE, ButtonScanShift[i]);
				if(GPIO_ReadInputDataBit( KEY_IN_DET_PORT, KEY_IN_DET_PIN)==1)
					KeyDebounceTime[i]++;
				else
					KeyDebounceTime[i]=0;
				
				if(KeyDebounceTime[i]==BUTTON_LONG_TIME)
				{
					//KeyDebounceTime[i] = 0;  //can clear it under 
					KeyValue |= (ButtonScanShift[i] | 0x80);
				}
			}
			HC595_Updata(SER_DOT_INTERFACE, 0x00);		
		}	
		else
		{
			for(i=0; i<KEY_NUM; i++)
			{
//				if(KeyDebounceTime[i]>=BUTTON_LONG_TIME)
//					KeyValue |= (ButtonScanShift[i] | 0x80);
				if((KeyDebounceTime[i]>=BUTTON_SHORT_TIME)&&(KeyDebounceTime[i]<BUTTON_LONG_TIME))
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
			ONE_WARM_BEEP();
			printf("KeyValue: %X\r\n", KeyValue);
		}
}


void WakeUp_Interrupt_Exti_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the BUTTON Clock */
	
  RCC_AHBPeriphClockCmd(KEY_IN_DET_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure Button pin as input */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = KEY_IN_DET_PIN;
  GPIO_Init(KEY_IN_DET_PORT, &GPIO_InitStructure);
	


    /* Connect Button EXTI Line to Button GPIO Pin */
	SYSCFG_EXTILineConfig(KEY_IN_DET_EXTI_PORT_SOURCE, KEY_IN_DET_EXTI_PIN_SOURCE);

	/* Configure Button EXTI line */
	EXTI_InitStructure.EXTI_Line = KEY_IN_DET_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;    
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	
	/* Enable and set Button EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = KEY_IN_DET_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure); 
	
	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
	NVIC_Init(&NVIC_InitStructure); 
}

void WakeUp_Interrupt_Exti_Disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* Enable and set Button EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = KEY_IN_DET_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
	NVIC_Init(&NVIC_InitStructure); 
}







