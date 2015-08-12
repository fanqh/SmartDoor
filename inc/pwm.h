#ifndef __PWM_H_
#define __PWM_H_
#include "stm32f0xx.h"
#include "74HC595.h"

void Beep_PWM_Init(void);
void Hal_Beep_Blink (uint32_t numBlinks, uint32_t ontime, uint32_t offtime);
#define ONE_WARM_BEEP()   {Hal_Beep_Blink (1, 20, 10);}
void Beep_Three_Time(void);
void Beep_Four_Time(void);
extern HalLedControl_t HalBeepControl;

void Beep_ON(void);
void Beep_OFF(void);

#endif


