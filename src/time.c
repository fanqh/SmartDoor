#include "time.h"
#include    "Link_list.h"

uint32_t SystemTime = 0;

void Time3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	  /* Set the default configuration */
    TIM_TimeBaseInitStruct.TIM_Period = 2-1; //2ms
    TIM_TimeBaseInitStruct.TIM_Prescaler = 48000 -1;  //ms
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);


	
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM3, ENABLE);	// 开启时钟   
}


void Time3_Process(void)
{
	SystemTime++;
	if(SystemTime%500==0)//每隔1s喂狗一次
	{
		IWDG_ReloadCounter();
		//printf("[TIME]: rload IWDG \r\n");
	}
}

uint32_t GetSystemTime(void)
{
	return SystemTime;
}

