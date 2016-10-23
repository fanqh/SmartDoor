#ifndef __FLASH_MANAGE_C_
#define __FLASH_MANAGE_C_
#include "stm32f0xx.h"
#include "process_event.h"

#define FLASH_LOCK_WORK_MODE_ADDR   0x0800DC00 
void Flash_Erase(uint32_t addr);
void Flash_Write(uint32_t addr, uint16_t value);

void Sytem_Mode_Write(uint8_t mode);
uint8_t Get_System_Mode(void);

#endif
