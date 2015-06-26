#include "adc.h"
#include "Link_list.h"
#include "delay.h"

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

static void Battey_Sample_Ctr_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}
static void Battey_Sample_Ctr_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);	
}


static void Battery_ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStruct;
	
	
	
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
	 
	 ADC_DiscModeCmd(ADC1, ENABLE);
	 ADC_WaitModeCmd(ADC1, ENABLE);
	   /* ADC Calibration */
  //ADC_GetCalibrationFactor(ADC1);
	
  /* Enable the ADC peripheral */
  ADC_Cmd(ADC1, ENABLE);    	
	   /* Wait the ADRDY flag */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)); 
	
//	  /* ADC1 regular Software Start Conv */ 
  ADC_StartOfConversion(ADC1);
 }

void Hal_Battery_Sample_Task_Register(void)
{
		Battery_ADC_Init();
		Battery_Sample_Ctr_GPIO_Config();
		ADC_StartOfConversion(ADC1);
	//  lklt_insert(&ADC_node, Battery_Process, NULL, 100/2);
}


void Battery_Process(void)
{
	uint16_t time;
	uint16_t adc_value;

	
		Battey_Sample_Ctr_ON();	
		ADC_StartOfConversion(ADC1);

		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
		{
			if(time>5000)
			{
				ADC_Cmd(ADC1, ENABLE); 
				ADC_Cmd(ADC1, DISABLE); 
				Battery_ADC_Init();
				break;
			}
				
			time++;
			delay_us(100);
		}
		if(time<5000)
		{
			/* Get ADC1 converted data */
			adc_value = ADC_GetConversionValue(ADC1);
			
			ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
			printf("vol= %d\r\n", adc_value);
			/* Compute the voltage */	
			//ADC1ConvertedVoltage = (adc_value *3300)/0xFFF;
		}	
		time = 0;

}
