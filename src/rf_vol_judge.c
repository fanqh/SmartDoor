#include "rf_vol_judge.h"
#include "string.h"

#define SAVE_DATA_NUM  4


uint32_t vlist[SAVE_DATA_NUM] ;

//uint16_t Get_RF_ReadCount(void);
void Write_RF_ReadCount(uint16_t count);

uint32_t GetMIneVol(uint32_t addrbase)
{
	uint32_t min=0xffffffff;
	uint8_t i;
	uint32_t addr;
	
	addr = addrbase;
	for(i=0;i<SAVE_DATA_NUM;i++)
	{
		vlist[i] = *(uint32_t*)addr;
		if(min>vlist[i])
			min = vlist[i];
		addr = addr+4;
	}
	return min;
}
uint8_t AnalyzeVol(uint32_t vol, uint32_t addr)
{
	uint8_t i;
	uint32_t m;
	
	m = GetMIneVol(FLASH_VOL_ADDR);
	
	if((m!=0xffffffff)&&(vol<(m*90/100)))  //сп©╗©©╫Э
	{
		return 1;
	}
	else
	{
//		count = Get_RF_ReadCount();
//		
//		if((count!=0xffff) &&(count>=3))
		{
			for(i=0;i<SAVE_DATA_NUM;i++)
			{
				vlist[i] = vlist[i+1];
			}
			vlist[SAVE_DATA_NUM-1] = vol;
			WriteVolToFlash(addr);
//			Write_RF_ReadCount(0);
		}
//		else
//		{
//			if(count==0xffff)
//				count = 0;
//			Write_RF_ReadCount(++count);
//		}
		return 0;
	}
}

//uint16_t GetReadCount(void)
//{
//	
//}




uint8_t WriteVolToFlash(uint32_t addrbase)
{
	uint8_t i;
	FLASH_Status ret;
	uint32_t addr;
	
	addr = addrbase;
	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
	FLASH_ErasePage(addr);
	for(i=0; i<SAVE_DATA_NUM ; i++)
	{
		ret = FLASH_ProgramWord(addr, vlist[i]);
		addr = addr + 4;
	}
	FLASH_Lock();	
//	printf("ret = %d\r\n", ret);
	return ret;
}


void EreaseAddrPage(uint32_t addr)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
	FLASH_ErasePage(addr);
	FLASH_Lock(); 	
}

//uint16_t Get_RF_ReadCount(void)
//{
//	return *(uint16_t*)FLASH_VOL_READ_ADDR;
//}

//void Write_RF_ReadCount(uint16_t count)
//{
//	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
//	FLASH_ErasePage(FLASH_VOL_READ_ADDR);
//	FLASH_ProgramHalfWord(FLASH_VOL_READ_ADDR, count);
//	FLASH_Lock();	

//}












