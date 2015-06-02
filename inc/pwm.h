#ifndef __PWM_H_
#define __PWM_H_
#include "stm32f0xx.h"

void Beep_PWM_Init(void);
void Hal_Beep_Blink (uint32_t numBlinks, uint32_t ontime, uint32_t offtime);

#endif


