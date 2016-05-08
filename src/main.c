/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "uart.h"
#include "stdio.h"
#include "delay.h"
//#include "rc52x_hal.h"
//#include "RF.h"
//#include "RF_Hardware.h"
//#include "spi.h"
#include "RF1356.h"
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
#include "index.h"
#include "debug.h"
#include "adc.h"
#include "motor.h"
#include "sleep_mode.h"
#include "string.h"
#include "lock_key.h"
#include "factory_mode.h"
#include "time.h"
#include "finger.h"
#include "lpcd_api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RF 1
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  
  */
//#define FINGER 1
uint8_t Button_Cancle_Flag = 0;
extern uint8_t factory_mode;
extern uint32_t SleepTime_End;
uint8_t vol_low_warm_flag = 0;
extern void RF_Scan_Fun(void *priv);
struct node_struct_t RF_Scan_Node;

static void Gpio_test_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
//	GPIO_SetBits(GPIOA, GPIO_Pin_15);
}


/*
iwdg 1s 
*/
void IWDG_init(void)
{
//	RCC_LSICmd(ENABLE);
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	IWDG_SetReload(40000/32);  //4s//5s
	IWDG_ReloadCounter();
	IWDG_Enable();
}


enum wakeup_source_t Get_WakeUp_Source(void)
{
	enum wakeup_source_t ret;
	
	if(PWR_GetFlagStatus(PWR_FLAG_WU)==SET)
	{
		if(RTC_GetITStatus(RTC_IT_ALRA)==SET)
		{
			ret = TICK_WAKEUP;
			//printf("TICK_WAKEUP\r\n");
		}
#ifdef FINGER	
		else if(is_finger_wakeup()==1)
		{
		   ret = FINGER_WAKEUP;
		   printf("finger_wakeup\r\n");
		}
#endif
		else if((mpr121_get_irq_status()==0)&&(GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)!=0))
		{
			ret = TOUCH_WAKEUP;
			printf("touch_wakeup\r\n");
		}
		else if((GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)==0)&&(mpr121_get_irq_status()==0))
		{
			ret = BUTTON_WAKEUP;
			printf("button_wakeup\r\n");
		}
		else if(card_irq_status()==0)
		{
			ret = RF_WAKEUP;
			printf("RF_wakeup \r\n");
		}
		else
		{
			ret = TOUCH_WAKEUP;
			printf("unknown wakeup source irq= %d\r\n", card_irq_status());
		}
	}
	else
	{
		ret = SYSTEM_RESET_WAKEUP;
		printf("system_reset_wakeup\r\n");
	}
	
	 return ret;
}

//input mode
//0: 系统重启
//1: 系统唤醒初始化
//3: 其他模式没有指示灯和蜂鸣器进入LOCK_INIT状态
void Init_Module(enum wakeup_source_t mode)
{
	uint16_t code;
	uint8_t state;
	
//	IWDG_ReloadCounter();
	if(mode==TICK_WAKEUP)
	{
		uint16_t retry =0;
		
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		Lock_EnterIdle1();
		while(retry<5000) {retry++;}
		return;
		
	}
	if(mode==SYSTEM_RESET_WAKEUP)
	{
		
		Funtion_Test_Pin_config();
		if(Get_Funtion_Pin_State()==0)   //进入测试模式
		{
			factory_mode = 1;
			factory_mode_procss();
		}
	}
	lklt_init();
	delay_init();	
	Beep_PWM_Init();           //1. beep	
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
	Hal_Battery_Sample_Task_Register();
//	Finger_LDO_Init();
//	Finger_RF_LDO_Enable();
	IIC_Init();
	mpr121_init_config();    //2. touch key
    Index_Init();
	
#ifdef FINGER
	finger_init();
#endif	
	
	//Beep_Battery_Low_Block();
	if(mode!=OTHER_WAKEUP)
	{
 ///应用程序需要打开
		uint32_t vol;

		vol = Get_Battery_Vol();
		printf("vol = %d\r\n", vol);
//		if(vol<=4400)
//		{
//			Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_RED_ON_VALUE);
//			Battery_Low_Warm();			
//			delay_ms(300);
//			Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);
//			Lock_EnterIdle();			
//		}	
//		else if(vol<=4800)
//		{
//			Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeBatteryLowlMode() );
//			Battery_Low_Warm();
//			vol_low_warm_flag = 1;
//		}

		Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
		Beep_Power_On();
	}

	if((mode==FINGER_WAKEUP) ||(mode==BUTTON_WAKEUP) || (mode==TOUCH_WAKEUP)||(mode==RF_WAKEUP))
	{
		if((Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG))
		{
			lock_operate.lock_state = LOCK_OPEN_NORMAL;
			Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);	
			PASSWD_SUCESS_ON();
			SleepTime_End = GetSystemTime() + 3000;
		}
		else
		{

			if(Get_id_Number()!=0)
			{
				 code = GetDisplayCodeActive();
				 Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code );
			}
			else
			{
				 code = GetDisplayCodeNull(); 
				 Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code ); 
				 Beep_Null_Warm_Block();
			}
			Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_EnterReady() );
		}
	}
	else if(mode==SYSTEM_RESET_WAKEUP)
	{
		if(Get_id_Number()!=0)
		{
			 code = GetDisplayCodeActive();
			 Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code );
		}
		else
		{
			 code = GetDisplayCodeNull(); 
			 Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code ); 
			 Beep_Null_Warm_Block();
		}
		lock_operate.lock_state = LOCK_CLOSE;
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeActive() );
		
#if RF
		RF1356_MasterInit();
		RF1356_RC523Init();                     //6.RF
		delay_ms(5);
		state = LPCD_IRQ_int();
		printf("state = %d\r\n",state);
		if(state==1)
		{
			RF1356_SET_RESET_LOW();
		//	delay_ms(5);
		}	
#endif		
		
		
		
	}
	else
		lock_operate.lock_state = LOCK_INIT;
#if RF
	RF1356_MasterInit();
//	RF1356_RC523Init();                     //6.RF
//	delay_ms(5);
//	state = LPCD_IRQ_int();
//	printf("state = %d\r\n",state);
//	if(state==1)
//	{
//		RF1356_SET_RESET_LOW();
//	//	delay_ms(5);
//	}		
	lklt_insert(&RF_Scan_Node, RF_Scan_Fun, NULL, 10*TRAV_INTERVAL); 
	if((mode==TOUCH_WAKEUP) || (mode==RF_WAKEUP))
	{
		printf("scan1......main.\r\n");
		RF_Scan_Fun(&code);
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
		process_event();   ///为了提高扫卡激活反应灵敏度
		printf("scan2......main.\r\n");
		
	}
#endif	
	Hal_SEG_LED_Init();	     //3.SEG_LED
	Hal_LED_Task_Register(); //4. LED
	
	if(mode==BUTTON_WAKEUP)
		Button_Cancle_Flag = 1;
	
	Button_Key_Init();               //7. button
//	Time3_Init();	
	Time14_Init();
	Motor_GPIO_Init();

	Process_Event_Task_Register();   //5.EVENT_TASK	
	if(mode==SYSTEM_RESET_WAKEUP)
	{
		
		if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  //看门狗检测
		{
			RCC_ClearFlag();
			printf("********iwdg reset***********\r\n");
		}
		if(Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG)
			Erase_Open_Normally_Mode();
		IWDG_init();
	}
	if((mode==FINGER_WAKEUP) ||(mode==BUTTON_WAKEUP) || (mode==TOUCH_WAKEUP)||(mode==SYSTEM_RESET_WAKEUP) ||(mode==RF_WAKEUP) )
	{
		if(GetLockFlag(FLASH_LOCK_FLAG_ADDR)!=0xffff)
			EreaseAddrPage(FLASH_LOCK_FLAG_ADDR);
	}
	if(Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG)
		SleepTime_End = GetSystemTime() + 3000;
			
}


		
int main(void)
{
	enum wakeup_source_t  wakeup_source;
	
	uart1_Init();
	
#ifdef FINGER	
	finger_wakeup_detect_pin_init();
#elif RF
	card_irq_init();
#endif
	Key_Lock_Pin_Init();
	mpr121_IRQ_Pin_Config();
	Button_KeyInDec_Gpio_Config();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	wakeup_source = Get_WakeUp_Source();
	
	
	Init_Module(wakeup_source);
	
	printf("hello ...\r\n");	
//	if((GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)==0)&&(mpr121_get_irq_status()==0))
//		printf("here....\r\n");
    while (1)
    {  	
		
		uint32_t time1;
		uint32_t time=0;
		time = GetTick();
		Finger_Scan();
//		RF_Scan_Fun(&time1);
		process_event();
		if((time!=time1))
		{
//			time1 = time;
//			touch_key_scan(&time);
			lklt_traversal();
		}	
    }

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
