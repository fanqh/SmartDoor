#ifndef __DELAY_H_
#define __DELAY_H_
#include "stm32f0xx.h"

void delay_init(void);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
void delay_s(uint16_t s);

#endif
