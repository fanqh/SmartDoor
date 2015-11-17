#include "rf_vol_judge.h"

uint32_t GetAverageVol(uint32_t addr)
{
	uint32_t vol;
	
	vol = *(uint32_t*)addr;
	return vol;
}

uint8_t WriteVolToFlash(uint32_t addr, uint32_t vol)
{
	FLASH_Status ret;
	
	FLASH_Unlock();
	ret = FLASH_ProgramWord(addr, vol);
	FLASH_Lock(); 	
	if(ret!=FLASH_COMPLETE)
		return 0;
	else
		return 1;
}


void EreaseAddrPage(uint32_t addr)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
	FLASH_ErasePage(addr);
	FLASH_Lock(); 	
}












