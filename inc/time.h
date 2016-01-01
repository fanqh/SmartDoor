#ifndef __TIME_H_
#define __TIME_H_
#include "stm32f0xx.h"

void Time3_Init(void);
void Time3_Process(void);
uint32_t GetSystemTime(void);
void Time14_Process(void);
void Time14_Init(void);


#endif

