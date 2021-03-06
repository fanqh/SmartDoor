#ifndef __LED_BLINK_H_
#define __LED_BLINK_H_

#include "stm32f0xx.h"

#define LED_ALL_OFF_VALUE  					(0xffff&(~((1UL<<14)|(1UL<<15))))
#define LED_BLUE_ALL_ON_VALUE   		(0x00)
#define LED_RED_ON_VALUE  					(0xffff&(~(1UL<<15)))
#define LED_GREEN_ON_VALUE          (0xffff&(~(1UL<<14)))
#define LED_BOTTERY_LOW_WARM_VALUE  ((uint16_t)1<<13)
#define LED_OPEN_NORMALLY_MODE			((uint16_t)1<<12)


uint16_t Random16bitdata(void);
void Hal_LED_Display_Set(uint8_t mode, uint16_t DisplayCode);
void Hal_LED_Task_Register(void);
void HAL_LED_Blue_ON_Contine(uint16_t code, uint32_t ontime);
void Hal_LED_Blink (uint16_t code, uint32_t numBlinks, uint32_t ontime, uint32_t offtime);
void Led_Battery_Low_ON(void);
void Led_Battery_Low_OFF(void);
void Led_Open_Normally_ON(void);
void Led_Open_Normally_OFF(void);

#endif
