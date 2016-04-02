#include "adc.h"
#include "Link_list.h"
#include "delay.h"
#include "process_event.h"
#include "time.h"
#include "uart.h"
#include "led_blink.h"
#include "pwm.h"


#define BATTERY_SAMPLE_TIME  3
#define SAMPLE_TIME      16//5
#define ADC1_DR_Address                0x40012440
__IO uint32_t TempSensVoltmv = 0, VrefIntVoltmv = 0;
__IO uint16_t RegularConvData_Tab[SAMPLE_TIME];

struct node_struct_t ADC_node;

//static void Battery_Process(void);

static void Battery_ADC_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;		 
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void Battery_Sample_Ctr_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
}

 void Battey_Sample_Ctr_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}
 void Battey_Sample_Ctr_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);	
}


 void Battery_ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStruct;

	uint16_t retry = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); 
	Battery_ADC_GPIO_Config();
	ADC_StructInit(&ADC_InitStruct);
	/* Reset ADC init structure parameters values */
	/* Initialize the ADC_Resolution member */
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;

	/* Initialize the ADC_ContinuousConvMode member */
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;

	/* Initialize the ADC_ExternalTrigConvEdge member */
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

	/* Initialize the ADC_ExternalTrigConv member */
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;

	/* Initialize the ADC_DataAlign member */
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;

	/* Initialize the ADC_ScanDirection member */
	ADC_InitStruct.ADC_ScanDirection = ADC_ScanDirection_Upward;

	ADC_DeInit(ADC1);

	ADC_ClockModeConfig(ADC1, ADC_ClockMode_SynClkDiv2);
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_ChannelConfig(ADC1, ADC_Channel_1 , ADC_SampleTime_55_5Cycles);

	//ADC_DiscModeCmd(ADC1, ENABLE);
	//ADC_WaitModeCmd(ADC1, ENABLE);
	/* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1);

	/* Enable the ADC peripheral */
	ADC_Cmd(ADC1, ENABLE);    	
	/* Wait the ADRDY flag */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)&&(retry<100))
	{
		retry++;
		delay_us(1);
	} 
	
//	  /* ADC1 regular Software Start Conv */ 
//  ADC_StartOfConversion(ADC1);
}

void Hal_Battery_Sample_Task_Register(void)
{
	Battery_ADC_Init();
	Battery_Sample_Ctr_GPIO_Config();
		
//		Battery_Process();
//	  lklt_insert(&ADC_node, Battery_Process, NULL, 10000/10);
}

void Battery_Low_Warm(void)
{
	uint8_t i;
	
	Beep_PWM_TimeBase_config(420); 
	Beep_PWM_config(200);	
	for(i=0;i<10;i++)
	{
		Beep_ON();
		HC595_ShiftOut16(SER_LED_INTERFACE, LED_RED_ON_VALUE);
		delay_ms(10);
		
		Beep_OFF();	
		HC595_ShiftOut16(SER_LED_INTERFACE, LED_ALL_OFF_VALUE);
		delay_ms(10);
	}
}	

void Beep_Bat_Low(void)
{   
	uint8_t i;
	Beep_PWM_TimeBase_config(420); 
	Beep_PWM_config(200);	
	for(i=0;i<10;i++)
	{
		Beep_ON();
		//HC595_ShiftOut16(SER_LED_INTERFACE, LED_RED_ON_VALUE);
		delay_ms(10);
		
		Beep_OFF();	
		//HC595_ShiftOut16(SER_LED_INTERFACE, LED_ALL_OFF_VALUE);
		delay_ms(10);
	}
}


/**
  * @brief  ADC1 channel with DMA configuration
  * @param  None
  * @retval None
  */
#if 1
void ADC1_CH_DMA_Config(void)
{
  ADC_InitTypeDef     ADC_InitStructure;
  DMA_InitTypeDef     DMA_InitStructure;
  GPIO_InitTypeDef    GPIO_InitStructure;
  
  /* ADC1 DeInit */  
 // ADC_DeInit(ADC1);
	  /* GPIOC Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* ADC1 Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
  /* DMA1 clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	
	
	  /* Configure ADC Channel2 as analog input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* DMA1 Channel1 Config */
 // DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RegularConvData_Tab;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = SAMPLE_TIME;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* DMA1 Channel1 enable */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC DMA request in circular mode */
  ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
  
  /* Enable ADC_DMA */
  ADC_DMACmd(ADC1, ENABLE);  
  
  /* Initialize ADC structure */
  ADC_StructInit(&ADC_InitStructure);
  
  /* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Backward;
  ADC_Init(ADC1, &ADC_InitStructure); 
 
   /* Convert the ADC1 Channel 11 with 239.5 Cycles as sampling time */ 
  ADC_ChannelConfig(ADC1, ADC_Channel_2 , ADC_SampleTime_7_5Cycles);    
 // ADC_TempSensorCmd(ENABLE);

  /* ADC Calibration */
  //ADC_GetCalibrationFactor(ADC1);
	
	  /* Enable the auto delay feature */    
  ADC_WaitModeCmd(ADC1, ENABLE); 
  
  /* Enable the Auto power off mode */
  ADC_AutoPowerOffCmd(ADC1, ENABLE); 
  
//  /* Enable ADC1 */
//  ADC_Cmd(ADC1, ENABLE);     
//  
//  /* Wait the ADCEN falg */
//  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)); 
  
  /* ADC1 regular Software Start Conv */ 
 // ADC_StartOfConversion(ADC1);
}

uint32_t Get_RF_Voltage(void)
{
	
	uint16_t i;
	uint32_t vol =0;
	uint16_t retry = 0;
	uint16_t max=0;
	  /* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);     

	/* Wait the ADCEN falg */
	while((!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN))&&(retry<100))
	{
		retry++;
		delay_us(1);
	}

	ADC_StartOfConversion(ADC1);
	retry = 0;
		/* Test DMA1 TC flag */
	while(((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET)&&(retry<100) )
	{
		retry++;
		delay_us(1);
	}

	/* Clear DMA TC flag */
	DMA_ClearFlag(DMA1_FLAG_TC1);
	//	ADC_Cmd(ADC1, DISABLE); 

//	printf("ad_vol= ");
	for(i=0;i<SAMPLE_TIME;i++)
	{
		if(max<RegularConvData_Tab[i])
			max = RegularConvData_Tab[i];
//		vol += RegularConvData_Tab[i];
//		printf("%d ", RegularConvData_Tab[i]*3300/0xfff);
	}
//	vol = vol*3300/(0xfff*SAMPLE_TIME);
//	printf("max = %d, max = %d\r\n", vol, max);
//	printf("\r\n");
	
	vol = max*3300/0xfff;
	return vol;
}
#else
void ADC1_CH_DMA_Config(void)
{
	ADC_InitTypeDef 				 ADC_InitStruct;
	GPIO_InitTypeDef         GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); 
		  /* Configure ADC Channel2 as analog input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_StructInit(&ADC_InitStruct);
	  /* Reset ADC init structure parameters values */
  /* Initialize the ADC_Resolution member */
  ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;

   /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;

  /* Initialize the ADC_ExternalTrigConvEdge member */
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;

  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;

  /* Initialize the ADC_ScanDirection member */
  ADC_InitStruct.ADC_ScanDirection = ADC_ScanDirection_Upward;
	
	ADC_DeInit(ADC1);
	
	ADC_ClockModeConfig(ADC1, ADC_ClockMode_SynClkDiv2);
	 ADC_Init(ADC1, &ADC_InitStruct);
	 ADC_ChannelConfig(ADC1, ADC_Channel_2 , ADC_SampleTime_239_5Cycles);
	 
	 //ADC_DiscModeCmd(ADC1, ENABLE);
	 //ADC_WaitModeCmd(ADC1, ENABLE);
	   /* ADC Calibration */
  ADC_GetCalibrationFactor(ADC1);
	
  /* Enable the ADC peripheral */
  ADC_Cmd(ADC1, ENABLE);    	
	   /* Wait the ADRDY flag */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)); 
}

uint32_t Get_RF_Voltage(void)
{
	uint16_t adc_value;
	
			ADC_StartOfConversion(ADC1);
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		/* Get ADC1 converted data */
		adc_value = ADC_GetConversionValue(ADC1);	
		ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
		
		return adc_value*3300/(0xfff-1);
	
}

#endif


uint16_t Get_Battery_Vol(void)
{
	uint16_t time;
	uint16_t adc_value;
	uint16_t vol,sum;
	uint16_t retry = 0;

	sum = 0;
	vol = 0;
	
	Battey_Sample_Ctr_ON();	
	delay_ms(1);
	ADC_StartOfConversion(ADC1);
	for(time=0; time<BATTERY_SAMPLE_TIME; time++)
	{
		ADC_StartOfConversion(ADC1);
		while((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)&&(retry<1000))
		{
			retry ++;
			delay_us(1);
		}
		/* Get ADC1 converted data */
		adc_value = ADC_GetConversionValue(ADC1);	
		ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
		
		vol = adc_value*3300/(0xfff-1);
		sum += vol;	
		delay_ms(2);
//		printf("vol%d= %d\r\n", time,adc_value);
	}
	vol = (sum*147)/(47*BATTERY_SAMPLE_TIME);
//	printf("bat= %d\r\n", lock_operate.BatVol);
//	if(lock_operate.BatVol<4500)
//	{
//		Battery_Low_Warm();
//	}

	ADC_Cmd(ADC1, DISABLE);  
	Battey_Sample_Ctr_OFF();	
	return vol;
}
