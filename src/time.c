#include "time.h"
#include    "Link_list.h"
#include "mpr121_key.h"
#include "process_event.h"

uint32_t SystemTime = 0;
static uint32_t key_time = 0;
uint32_t tick = 0;

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
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
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




void Time14_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	  /* Set the default configuration */
    TIM_TimeBaseInitStruct.TIM_Period = 200-1; //200ns
    TIM_TimeBaseInitStruct.TIM_Prescaler =  48 -1;  //1ns
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStruct);


	
    NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_ClearFlag(TIM14, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM14, ENABLE);	// 开启时钟   
	
}


void Time14_Process(void)
{

	
	key_time++;
	tick++;
	touch_key_scan(&key_time);
//	lklt_traversal();
///	process_event();
}

uint32_t GetKeyTime(void)
{
	return key_time;
}
uint32_t GetTick(void)
{
	return key_time;
}

