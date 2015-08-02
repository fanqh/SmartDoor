#ifndef __ADC_H_
#define __ADC_H_

#include "stm32f0xx.h"

void Hal_Battery_Sample_Task_Register(void);
void Battery_Process(void);
 void Battey_Sample_Ctr_ON(void);
 void Battey_Sample_Ctr_OFF(void);
 void ADC1_CH_DMA_Config(void);
 uint32_t Get_RF_Voltage(void);

#endif

