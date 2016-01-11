/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32f0xx_it.h"
#include "time.h"
#include "stm32f0xx.h"
#include "button_key.h"
#include "process_event.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


void TIM3_IRQHandler(void)
{
	
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{
				TIM_ClearFlag(TIM3, TIM_FLAG_Update);	// 清除溢出中断标志 
				Time3_Process();
		}
}

void TIM14_IRQHandler(void)
{
	
		if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
		{
			TIM_ClearFlag(TIM14, TIM_FLAG_Update);	// 清除溢出中断标志 
			Time14_Process();
		}
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE)!=RESET)
	{
		USART_ClearFlag(USART1, USART_IT_RXNE);
		urart_rec(USART_ReceiveData(USART1));
		
	}
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles RTC Auto wake-up interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {

		printf("wakeupPin is work\r\n");
    /* Clear the Alarm A Pending Bit */
    RTC_ClearITPendingBit(RTC_IT_WUT);
    
    /* Clear EXTI line17 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line17);    
  }
	else if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
	{
		printf("alrmA is work\r\n");
		RTC_ClearITPendingBit(RTC_IT_ALRA);
	}
  
}

/**
  * @brief  This function handles External lines 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_15_IRQHandler(void)
{
//  if(EXTI_GetITStatus(KEY_IN_DET_EXTI_LINE) != RESET)
//  { 
//		WakeupFlag |= 0x01;
//    /* Clear the USER Button EXTI line pending bit */
//    EXTI_ClearITPendingBit(KEY_IN_DET_EXTI_LINE);
////		SytemWakeup();

//  }
}


/**
  * @brief  This function handles External lines 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_3_IRQHandler(void)
{
//  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
//  { 
//		WakeupFlag |= 0x01;
//    /* Clear the USER Button EXTI line pending bit */
//    EXTI_ClearITPendingBit(EXTI_Line2);

//  }
}

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
