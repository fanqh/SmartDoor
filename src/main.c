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
	RF_Spi_Config();
	Time3_Init();
 
  Process_Event_Task_Register();   //5.EVENT_TASK

	RF_Init();                       //6.RF
	Button_Key_Init();               //7. button
	
	Motor_GPIO_Init();	
	Hal_Battery_Sample_Task_Register();
	Motor_Init();	
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);  //如果不加，，Bat低会把所有灯熄灭
	lock_operate.lock_state = LOCK_READY;
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeActive() );	
}	
		
int main(void)
{
	uint32_t RF_Vol =0;  

//	uart1_Init();
	mpr121_IRQ_Pin_Config();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);

	if(PWR_GetFlagStatus(PWR_FLAG_WU)==SET)
	{
		if(!(mpr121_get_irq_status()))
		{
				printf("key wakeup\r\n");
				Main_Init();
				Touch_Once__Warm();

		}
		else 
		{	
			delay_init();
			ADC1_CH_DMA_Config();	
			RF_Spi_Config();
			
			RF_PowerOn();
			RF_TurnON_TX_Driver_Data();
			RF_Vol = Get_RF_Voltage();
			//printf("vol=%dmV\r\n", RF_Vol);
			if(RF_Vol<140)
			{
					Main_Init(); 
					Touch_Once__Warm();
					printf("card wakeup...........\r\n"); 
			}
			else
			{
				uint16_t retry =0;
				
				Lock_EnterIdle1();
				while(retry<100) {retry++;delay_us(1);}
			}
		}
	}
	else
	{
		Main_Init();
		Touch_Once__Warm();
		lock_operate.lock_state = LOCK_CLOSE;
		printf("power on\r\n");
	}
//		Lock_EnterIdle();
	
  while (1)
  {	
		uint32_t time1,time2;
		uint32_t time=0;
		time = GetSystemTime();
		
		
			if((time!=time1))
			{
					time1 = time;
					touch_key_scan(&time);
					lklt_traversal();
			}

#if 0

	  if((time%50==0)&&(time!=time2))
		{
			time2 = time;
			RF_Vol = Get_RF_Voltage();
			printf("vol = %d,time = %dms\r\n", RF_Vol, time);
			if(RF_Vol<30)
			{
				printf("vol = %d\r\n", RF_Vol);
//				memset(cardNum, 0, 4);
//				if(RF_GetCard(&cardType,cardNum)==MI_OK)
//				{
//					num++;
//					printf("%d, sucess\r\n", num);
//				}
//				else
//					printf("fail\r\n");
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
