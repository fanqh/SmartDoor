#ifndef __RF1356_H_
#define __RF1356_H_

#include "main.h"
#include "global.h"
#include "lpcd_api.h"

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_spi.h"


unsigned char RF1356_RC523Init(void);
int16_t RF1356_GetCard(void);
void RTC_WakeUp_GetCard(void);

void Rf1356_InitGetCard(void);
void RF1356_SleepCheckCard(void);
void card_irq_init(void);
uint8_t card_irq_status(void);
								   								  

#endif

