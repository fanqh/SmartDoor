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
/** @addtogroup STM32F0xx_StdPeriph_Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
	
	
			uint16_t temp;

			
			
			
void Main_Init(void)
{

	
	uart1_Init();
  delay_init();
	lklt_init();
	
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
	Hal_LED_Task_Register();
	Button_Key_Init();
	Hal_SEG_LED_Init();	
	
	RF_Spi_Config();
	Time3_Init();
	IIC_Init();
	mpr121_init_config();
	
	Beep_PWM_Init();
	Index_Init();
	Process_Event_Task_Register();
	Motor_GPIO_Init();
	RF_Init();

}	
void SystemPowerOn(void)
{
		uint16_t code;
	
	Hal_Battery_Sample_Task_Register();
	Hal_Beep_Blink (2, 100, 50);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
	
	if(Get_id_Number()!=0)
		 code = GetDisplayCodeActive();
	else
	{
		 code = GetDisplayCodeNull();   
	}
	lock_operate.lock_state = LOCK_READY;
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, code );	
}
			
			
int main(void)
{


	uint8_t cardType =0;
	uint8_t cardNum[4];
	uint8_t i = 0;
	uint16_t num=0;
	uint32_t RF_Vol =0;

#if 1
	uart1_Init();
	delay_init();
	mpr121_IRQ_Pin_Config();
	//Button_KeyInDec_Gpio_Config();
	Time3_Init();
	//ADC1_CH_DMA_Config();

//	if(mpr121_get_irq_status()==0)
//		printf("touch wakeup\r\n");
//	if(Get_Key_In0_Status()==0)	
//		printf("button key wakeup\r\n");
	if(!(mpr121_get_irq_debounce()))
	{
			printf("key wakeup\r\n");
		  Main_Init(); 
			SystemPowerOn();
	}
	else 
	{	
		ADC1_CH_DMA_Config();
		RF_Vol = Get_RF_Voltage();
		printf("vol=%dmV\r\n", RF_Vol);
		Main_Init(); 
		if(RF_GetCard(&cardType,cardNum)==MI_OK)
		{
			  printf("card\r\n"); 
				SystemPowerOn();
		}
		else
		{
			Lock_EnterIdle();
			while(1);
		}
	}
#endif
//	Motor_Init();	
	printf("system is work\r\n");
	
	//RF_Vol = Get_RF_Voltage();
	//printf("%d\r\n", RF_Vol);	
  while (1)
  {	
		uint8_t flag;
		uint32_t time1,time2;
		uint32_t time=0;
		time = GetSystemTime();
		
		
		if((time!=time1))
		{
			  time1 = time;
			//	printf("Time=%d\r\n",time);
				lklt_traversal();
		}
		

		
	 

#if 0

	  if((time%500==0)&&(time!=time2))
		{
			time2 = time;
			RF_Vol = Get_RF_Voltage();
			printf("vol = %d\r\n", RF_Vol);
			if(RF_Vol<30)
			{
				printf("vol = %d\r\n", RF_Vol);
				memset(cardNum, 0, 4);
				if(RF_GetCard(&cardType,cardNum)==MI_OK)
				{
					num++;
					printf("%d, sucess\r\n", num);
				}
				else
					printf("fail\r\n");
			}
		}
#endif
		
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
