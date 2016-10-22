#include "flash_manage.h"



void Flash_Write(uint32_t addr, uint16_t value)
{
	FLASH_Unlock();
	FLASH_ErasePage(addr);
	FLASH_ProgramHalfWord((uint32_t)addr, value);
	FLASH_Lock();
}

void Flash_Erase(uint32_t addr)
{
	FLASH_Unlock();
	FLASH_ErasePage(addr);
	FLASH_Lock();
}

uint8_t Get_System_Mode(void)
{
	uint8_t mode = 0;
	
	mode = (*(uint16_t*)FLASH_LOCK_WORK_MODE_ADDR) & 0xff;
	if(mode<=SYSTEM_MODE4)
		return mode;
	else
	{
		Flash_Write(FLASH_LOCK_WORK_MODE_ADDR, SYSTEM_MODE1);
		return SYSTEM_MODE1;
	}
}


void Sytem_Mode_Write(uint8_t mode)
{
	if(mode>SYSTEM_MODE4)
		mode = SYSTEM_MODE1;
	
	Flash_Write(FLASH_LOCK_WORK_MODE_ADDR, mode);
}
