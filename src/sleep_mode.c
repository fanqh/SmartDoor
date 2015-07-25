#include "sleep_mode.h"


void Gpio_Config_In_SleepMode(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF,ENABLE);
	
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		        
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
//	GPIO_Init(GPIOF, &GPIO_InitStruct);	
	  
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
//	  GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_6);		//|GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
//	GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_7|GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;	
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	
}

