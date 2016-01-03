#ifndef __SEG_LED_H_
#define __SEG_LED_H_

#include "stm32f0xx.h"
#include "stdbool.h"

bool Seg_LED_Dislay(uint8_t value);
extern uint8_t LEDDisplayCode[];

void Hal_SEG_LED_Init(void);
void Hal_SEG_LED_Display(uint16_t code);
//void Hal_SEG_LED_Blink (uint16_t code, uint32_t numBlinks, uint32_t ontime, uint32_t offtime);
//void Hal_SEG_LED_Blink_Stop(void);
void Hal_SEG_LED_Output_PreState(void);
void Hal_SEG_LED_Display_Set(uint8_t mode, uint16_t DisplayCode);
int16_t GetDispalayCodeFromValue(uint8_t value);

#endif

