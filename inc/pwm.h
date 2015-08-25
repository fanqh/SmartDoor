#ifndef __PWM_H_
#define __PWM_H_
#include "stm32f0xx.h"
#include "74HC595.h"

void Beep_PWM_Init(void);
void Key_Touch_Beep_Warm(void);
void Beep_PWM_TimeBase_config(uint16_t period);
void Beep_PWM_config(uint16_t pulse);
void Hal_Beep_Blink (uint32_t numBlinks, uint32_t ontime, uint32_t offtime);
void Beep_Three_Time(void);
void Beep_Four_Time(void);
extern HalLedControl_t HalBeepControl;
#define ONE_WARM_BEEP() {Key_Touch_Beep_Warm();} //{Beep_Once_Time();}//Hal_Beep_Blink (1, 20, 10);
void Beep_Once_Time(void);

void Beep_Compare_ID_Err(void);
void Beep_Bit_More(void);
void Beep_ON(void);
void Beep_OFF(void);

void Music_PWM(void);
void Beep_Two_Time(void);



void Beep_Register_OK_Block(void);


/**
* block function
*/
void Beep_Null_Warm_Block(void);

#endif


