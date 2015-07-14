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
#include "rc52x_hal.h"
#include "spi.h"
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
	//Disable_LDO();
	SpiMsterGpioInit();
	RF1356_RC523Init();
	IIC_Init();
	mpr121_init_config();
  Time3_Init();
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
	Button_Key_Init();
	Hal_SEG_LED_Init();
	Hal_LED_Task_Register();
	Beep_PWM_Init();
	Hal_Battery_Sample_Task_Register();
	Process_Event_Task_Register();
	Hal_Beep_Blink (2, 100, 50);
	Index_Init();
	Motor_GPIO_Init();
	Hal_LED_Display_Set(HAL_LED_MODE_FLASH, LED_BLUE_ALL_ON_VALUE);
	
}	
			
			
int main(void)
{

	uint16_t code;
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f0xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
     */ 
	 delay_init();
		IIC_Init();
	mpr121_init_config();
	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
		HC595_Power_OFF();
	//ADC_Cmd(ADC1, DISABLE); 
	mpr121_enter_standby();
		WakeUp_Interrupt_Exti_Config();

	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
#if 0	
	
  Main_Init();   
	if(Get_id_Number()!=0)
		 code = GetDisplayCodeActive();
	else
	{
		 code = GetDisplayCodeNull();   
//		 Hal_Beep_Blink (2, 50, 50);
	}
	lock_operate.lock_state = LOCK_READY;
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, code );
	Motor_Init();	
//	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
//	delay_ms(100);
  /* Add your application code here
     */
//	uart1_Init();
//	//LDO_Ctrl_Gpio_Init();
//	delay_init();
//	lklt_init();
//	//Disable_LDO();
//	SpiMsterGpioInit();
//	RF1356_RC523Init();
//	IIC_Init();
//	mpr121_init_config();
//  Time3_Init();
//	HC595_init(SER_LED_INTERFACE | SER_DOT_INTERFACE);
//	Button_Key_Init();
//	Hal_SEG_LED_Init();
//	Hal_LED_Task_Register();
//	Beep_PWM_Init();
//	Hal_Battery_Sample_Task_Register();
//	Process_Event_Task_Register();
//	Hal_Beep_Blink (2, 100, 50);
//	Index_Init();
//	Motor_GPIO_Init();
//	Motor_Drive_Forward();
	
//	Led_Battery_Low_ON();
//		HC595_ShiftOut(SER_LED_INTERFACE, 0x55);
//Hal_LED_Display_Set(HAL_LED_MODE_ON,0x00);
//HAL_LED_Blue_ON_Contine(LED_BLUE_ALL_ON_VALUE, 5000);

//  Hal_LED_Display_Set(HAL_LED_MODE_BLINK, 0x5555);//0xffff &(~((1UL<<6)|(1UL<<7)))
	//TIM_CtrlPWMOutputs(TIM16, ENABLE);
//	Hal_SEG_LED_Blink (0xf9a4, 0, 200, 200);
	//HAL_SEG_LED_Display(0xf9a4);
	//Seg_LED_Dislay(3);
	printf("system is work\r\n");
  /* Infinite loop */
  while (1)
  {	uint8_t flag;
		uint32_t time,time1;
		time = GetSystemTime();
//		if((time%5==0))
		{
//				time1 = time;
			//	printf("Time=%d\r\n",time);
				lklt_traversal();
		}
//		if(time%5000==0)		
//		{
//			Battery_Process();
//		}
//		delay_ms(500); 

#if 0
//		RF1356_MasterWriteData(0x17,0x22);
//		temp = RF1356_MasterReadData(0x17);
//		printf("temp = %X\r\n", temp);
//		delay_ms(500);
//		temp = RF1356_MasterReadData(0x37);
//		printf("temp = %X\r\n", temp);
//		delay_ms(500);
	if((time%5==0)&&(time!=time1))
	{
		time1 = time;
		RF1356_GetCard();
	}
#endif
  }
	
#endif	

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
