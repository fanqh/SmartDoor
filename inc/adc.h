#ifndef __ADC_H_
#define __ADC_H_

#include "stm32f0xx.h"

void Hal_Battery_Sample_Task_Register(void);
uint16_t Get_Battery_Vol(void);
 void ADC1_CH_DMA_Config(void);
 uint32_t Get_RF_Voltage(void);
 void Battery_Low_Warm(void);

#endif

