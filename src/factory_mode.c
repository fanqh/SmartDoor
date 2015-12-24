#include "stm32f0xx.h"
#include "main.h"
#include "uart.h"
#include "stdio.h"
#include "delay.h"
//#include "rc52x_hal.h"
#include "RF.h"
#include "RF_Hardware.h"
#include "iic.h"
#include "mpr121_key.h"
#include "time.h"
#include "Link_list.h"
#include "74HC595.h"
#include "button_key.h"
#include "seg_led.h"
#include "led_blink.h"
#include "pwm.h"
#include "process_event.h"
#include "motor.h"
#include "string.h"
#include "rf_vol_judge.h"
#include "lock_key.h"


enum TestCase_t
{
	UNTEST,
	SEG1,
	SEG2,
	RED_LED,
	GREEN_LED,
	BLUE_LED,
	LED_ALL_OFF,
	MOTOR_ON,
	MOTOR_OFF,
	RF_SCAN,
	SEG1_OFF,
	SEG2_OFF
};


void Funtion_Test_Pin_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

uint8_t Get_Funtion_Pin_State(void)
{
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
}







void factory_mode_procss(void)
{
	enum TestCase_t test_case;
	uint8_t seg1,seg2,segnum;
	uint16_t segcode;
	uint8_t green,red,blue, ledflag;
	uint8_t motorflag;

	
	lklt_init();
	delay_init();
	Beep_PWM_Init();           //1. beep
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
	
	IIC_Init();
	mpr121_init_config();    //2. touch key
	Hal_SEG_LED_Init();	     //3.SEG_LED
	Hal_LED_Task_Register(); //4. LED
	RF_Spi_Config();
	Time3_Init();
	RF_Init();                       //6.RF
//	RF_PowerOn();
//	RF_TurnON_TX_Driver_Data();
	Button_Key_Init();               //7. button
	
	Motor_GPIO_Init();	
	Motor_Init();
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, 0 );
	Touch_Once__Warm();
	IWDG_init();
	test_case = UNTEST;
	seg1 = 1;
	while(1)
	{
		static uint32_t time,time1,t3;
		Hal_EventTypedef e; 
	
		time= GetSystemTime();
		
		if((time!=time1))
		{
			time1 = time;
			lklt_traversal();
		
			e = USBH_GetEvent();
			if(e.event == TOUCH_KEY_EVENT)
			{
				switch(e.data.KeyValude)
				{
					case '1':
						seg1 = !seg1;
						if(test_case!=SEG1)
						{	
							seg1 = 1;
							test_case = SEG1;
						}
						
						break;
					case '2':
						seg2 = !seg2;
						if(test_case!=SEG2)
						{
							seg2 = 1;
							test_case = SEG2;
						}
						
						break;
					case '3':
						ledflag = !ledflag;
						if(test_case!=RED_LED)
						{
							ledflag = 1;
							test_case=RED_LED;
						}
						
						break;
					case '4':
						ledflag = !ledflag;
						if(test_case!=GREEN_LED)
						{
							ledflag = 1;
							test_case=GREEN_LED;
						}
						
						break;
					 case '5':
						ledflag = !ledflag;
						if(test_case!=BLUE_LED)
						{
							ledflag = 1;
							test_case=BLUE_LED;
						}
					 case '6':
						if(test_case!=LED_ALL_OFF)
							test_case=LED_ALL_OFF;
						break;
					 case '7':
						if(test_case!=MOTOR_ON)
							test_case=MOTOR_ON;
						motorflag = 1;
						break;
					 case '8':
						if(test_case!=MOTOR_OFF)
							test_case=MOTOR_OFF;
						motorflag = 1;
						break;
					 case '9':
						if(test_case!=RF_SCAN)
						{
							RF_PowerOn();
						    RF_TurnON_TX_Driver_Data();
							test_case=RF_SCAN;
						}
						break;
					case '*':
						if(test_case!=SEG1_OFF)
						{
							test_case=SEG1_OFF;
						}
						break;
					case '0':
						if(test_case!=SEG2_OFF)
						{
							test_case=SEG2_OFF;
						}
						break;
				}
			}
			
			switch(test_case)
			{
				case SEG1:	
					if(seg1==1)
					{
						if(time -t3>=300)
						{
							t3 = time;
							segnum = segnum%9 + 1;
							segcode = GetDispalayCodeFromValue(segnum);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, segcode) ;
						}	
					}
					else
					{
						segcode = 0;
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0);
					}
					break;
				case SEG2:	
					if(seg2==1)
					{
						if(time -t3>=300)
						{
							t3 = time;
							segnum = segnum%9 + 1;
							segcode = (GetDispalayCodeFromValue(segnum) <<8) |0xc0;
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, segcode);
						}	
					}
					else
					{
						segcode = 0;
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0);
					}
					break;
				case RED_LED:
					if(ledflag==1)
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_RED_ON_VALUE);
					else
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_ALL_OFF_VALUE);
					break;
				case GREEN_LED:
					if(ledflag==1)
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_GREEN_ON_VALUE);
					else
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_ALL_OFF_VALUE);
					break;
				case BLUE_LED:
					if(ledflag==1)
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
					else
						Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_ALL_OFF_VALUE);
					break;
				case LED_ALL_OFF:
					Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_ALL_OFF_VALUE);
					break;
				case MOTOR_ON:
					motorflag = 0;
					Motor_Drive_Forward();
					delay_ms(800);
					Motor_Drive_Stop();
					break;
				case MOTOR_OFF:
					motorflag = 0;
					Motor_Drive_Reverse();
					delay_ms(800);
					Motor_Drive_Stop();
					break;
				case RF_SCAN:
				{
				uint8_t cardType =0;
				uint8_t cardNum[5];
				uint8_t i;
				if(RF_GetCard(&cardType,cardNum)==MI_OK)
				{
					char null[4]= {0,0,0,0};
					cardNum[4]='\0';
					if(strcmp(cardNum, null)!=0)
					{
							Hal_Beep_Blink (1, 80, 30);
						
							printf("cardNum: \r\n");
							for(i=0;i<4;i++)
							{
								printf("%X",cardNum[i]);
							}
							printf("\r\n");
					}
				}
				}
				break;
				case SEG1_OFF:
					segcode = segcode | 0xff;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, segcode) ;
				break;
				case SEG2_OFF:
					segcode = segcode | 0xff00;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, segcode) ;
				break;
//				SEG2,
//				RED_LED,
//				GREEN_LED,
//				BLUE_LED,
//				LED_ALL_OFF,
//				MOTOR_ON,
//				MOTOR_OFF,
//				RF_SCAN,
//				SEG1_OFF,
//				SEG2_OFF
			}
			
			
		}
	}
	
	
	
}



