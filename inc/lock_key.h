#ifndef __LOCK_KEY__
#define __LOCK_KEY__

#include "stm32f0xx.h"

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_LOCK_FLAG_PAGE			57
//#define LOCK_FLAG    0x5566

void Key_Lock_Pin_Init(void);
uint8_t Get_Lock_Pin_State(void);
//void Lock_EreaseAddrPage(uint32_t addr);
uint16_t GetLockFlag(uint32_t addr);
uint8_t WriteLockFlag(uint32_t addr, uint16_t flag);


#endif


