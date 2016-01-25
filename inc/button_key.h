#ifndef __BUTTON_KEY_H_
#define __BUTTON_KEY_H_

#include "stm32f0xx.h"
#include "Link_list.h"

//#define 	KEY_CANCEL_SHORT	 1
//#define 	KEY_DEL_SHORT 		 2
//#define 	KEY_OK_SHORT  		 4
//#define 	KEY_INIT_SHORT 		 8
//#define 	KEY_ADD_SHORT   	0x10


#define 	KEY_CANCEL_SHORT	 1
#define 	KEY_ADD_SHORT 		 2
#define 	KEY_DEL_SHORT  		 4
#define 	KEY_INIT_SHORT 		 8
#define 	KEY_OK_SHORT   	0x10


#define 	KEY_CANCEL_LONG	 KEY_CANCEL_SHORT |0x80
#define 	KEY_DEL_LONG 		 KEY_DEL_SHORT |0x80
#define 	KEY_OK_LONG  		 KEY_OK_SHORT |0x80
#define 	KEY_INIT_LONG 	 KEY_INIT_SHORT |0x80
#define 	KEY_ADD_LONG   	 KEY_ADD_SHORT |0x80


#define 	KEY_IN_DET_PORT 	GPIOA
#define 	KEY_IN_DET_PIN  	GPIO_Pin_11
#define   KEY_IN_DET_CLK    RCC_AHBPeriph_GPIOA


#define KEY_IN_DET_EXTI_LINE          EXTI_Line11
#define KEY_IN_DET_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define KEY_IN_DET_EXTI_PIN_SOURCE    EXTI_PinSource11
#define KEY_IN_DET_EXTI_IRQn          EXTI4_15_IRQn 

void Button_Key_Init(void);  //TODO 以后要改成中断方式
void WakeUp_Interrupt_Exti_Config(void);
void WakeUp_Interrupt_Exti_Disable(void);
void Button_KeyInDec_Gpio_Config(void);
uint8_t Get_Key_In0_Status(void);
void Button_Key_Scan(void *priv);

#endif

