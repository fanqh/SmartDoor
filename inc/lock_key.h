#ifndef __LOCK_KEY__
#define __LOCK_KEY__

#include "stm32f0xx.h"

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_LOCK_FLAG_PAGE			57
#define FLASH_LOCK_FLAG_ADDR			0x0800E400
//#define LOCK_FLAG    0x5566

#define ERROR_STATE_TIMECOUNT_ADDR          0x0800E000

void Key_Lock_Pin_Init(void);
uint8_t Get_Lock_Pin_State(void);
//void Lock_EreaseAddrPage(uint32_t addr);
uint16_t GetLockFlag(uint32_t addr);
uint8_t WriteLockFlag(uint32_t addr, uint16_t flag);
void Lock_EreaseAddrPage(uint32_t addr);
void EreaseAddrPage(uint32_t addr);

#endif


