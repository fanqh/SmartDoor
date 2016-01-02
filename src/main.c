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
#include "RF.h"
#include "RF_Hardware.h"
//#include "spi.h"
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
#include "rf_vol_judge.h"
#include "lock_key.h"
#include "factory_mode.h"
#include "time.h"

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
	IWDG_SetReload(40000/32);  //4s
	IWDG_ReloadCounter();
	IWDG_Enable();
}

	
void Main_Init(void)
{		
	uint16_t code;
	
	lklt_init();
	delay_init();
	Index_Init();
	
	Beep_PWM_Init();           //1. beep
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
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
	
	

	IIC_Init();
	mpr121_init_config();    //2. touch key
	Hal_SEG_LED_Init();	     //3.SEG_LED
	Hal_LED_Task_Register(); //4. LED
#if RF
	RF_Spi_Config();
#endif
	Time3_Init();
 
	Process_Event_Task_Register();   //5.EVENT_TASK
#if RF
	RF_Init();                       //6.RF
//	RF_PowerOn();
//	RF_TurnON_TX_Driver_Data();
#endif
	Button_Key_Init();               //7. button
	
	Motor_GPIO_Init();	
	Time14_Init();
	Hal_Battery_Sample_Task_Register();
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);  //如果不加，，Bat低会把所有灯熄灭
	lock_operate.lock_state = LOCK_READY;
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeActive() );	

}	


		
int main(void)
{
	uint32_t RF_Vol =0;  
//	uint32_t min = 0;
	uart1_Init();
	
	mpr121_IRQ_Pin_Config();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	Key_Lock_Pin_Init();
	
	if(PWR_GetFlagStatus(PWR_FLAG_WU)==SET)
	{
		IWDG_ReloadCounter();
		if(!(mpr121_get_irq_status()))
		{
			uint8_t t1=0;

			printf("\r\n***key wakeup***\r\n");
			Main_Init();
			if(Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG)
			{
				lock_operate.lock_state = LOCK_OPEN_NORMAL;
				Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeOpenNormalMode());	
				
			}	
				
			if(GetLockFlag(FLASH_LOCK_FLAG_ADDR)!=0xffff)
				EreaseAddrPage(FLASH_LOCK_FLAG_ADDR);
			Touch_Once__Warm();
			Battery_Process();
			while(!mpr121_get_irq_status()&&(t1<100))
			{
				t1++;
				delay_ms(1);
				//printf("key is holding, please release the key\r\n");
			}

		}
		else 
		{	
			delay_init();	
#if RF	
//			RF_Spi_Config();		
//			RF_PowerOn();
//			RF_TurnON_TX_Driver_Data();
//			//delay_us(500);
//			ADC1_CH_DMA_Config();
//			RF_Vol = Get_RF_Voltage();
//#if 1
//		    printf("vol=%dmV\r\n", RF_Vol);
//			if(AnalyzeVol(RF_Vol,FLASH_VOL_ADDR)!=0)
//				

////			if((RF_Vol>(average*RF_VOL_WAKEUP_PERCENT_MIN))&&(RF_Vol<average*RF_VOL_WAKEUP_PERCENT_MAX))
//			{
//				Main_Init(); 
//				if(Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG)
//				{
//					lock_operate.lock_state = LOCK_OPEN_NORMAL;
//					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeOpenNormalMode());	
//					
//				}	
//				if(GetLockFlag(FLASH_LOCK_FLAG_ADDR)!=0xffff)
//					EreaseAddrPage(FLASH_LOCK_FLAG_ADDR);
//				Touch_Once__Warm();
//				Battery_Process();
//				printf("\r\n***card wakeup %dmV***\r\n", RF_Vol); 
//			}

//			else
//#endif
#endif
			{
				uint16_t retry =0;
				
				Lock_EnterIdle1();
				while(retry<100) {retry++;delay_us(1);}
			}
		}	
		
	}
	else
	{
		Funtion_Test_Pin_config();
		if(Get_Funtion_Pin_State()==0)
			factory_mode_procss();
		Main_Init();
		if(Get_Open_Normal_Motor_Flag()==LOCK_MODE_FLAG)
			Erase_Open_Normally_Mode();
		printf("power on\r\n");
		if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
		{
			RCC_ClearFlag();
			printf("iwdg reset\r\n");
		}
		if(GetLockFlag(FLASH_LOCK_FLAG_ADDR)!=0xffff)
			EreaseAddrPage(FLASH_LOCK_FLAG_ADDR);
		Touch_Once__Warm();
		lock_operate.lock_state = LOCK_CLOSE;
//		EreaseAddrPage(FLASH_VOL_ADDR);
		Battery_Process();
		IWDG_init();
	}
	
    while (1)
    {  	
		uint32_t time1,time2;
		uint32_t time=0;
		time = GetSystemTime();
		

		
		if((time!=time1))
		{
			time1 = time;
//			touch_key_scan(&time);
			lklt_traversal();
		}	
    }

}


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{  
  /* After wake-up from STOP reconfigure the system clock */
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {}
  
  /* Enable PLL */
  RCC_PLLCmd(ENABLE);
  
  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {}
  
  /* Select PLL as system clock source */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
  /* Wait till PLL is used as system clock source */
  while (RCC_GetSYSCLKSource() != 0x08)
  {}
}

void LowPower_Enter_Gpio_Config(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable the GPIOA peripheral */ 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOF, ENABLE);
  
  /* Configure MCO pin(PA8) in alternate function */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOF, &GPIO_InitStructure);
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
