#ifndef __LED_BLINK_H_
#define __LED_BLINK_H_

#include "stm32f0xx.h"

#define LED_ALL_OFF_VALUE  (0xffff&(~((1UL<<14)|(1UL<<15))))
#define LED_BLUE_ALL_ON_VALUE   	0x00
#define LED_RED_ON_VALUE  


uint16_t Random16bitdata(void);
void Hal_LED_Display_Set(uint8_t mode, uint16_t DisplayCode);
void Hal_LED_Task_Register(void);
void HAL_LED_Blue_ON_Contine(uint16_t code, uint32_t ontime);

#endif
