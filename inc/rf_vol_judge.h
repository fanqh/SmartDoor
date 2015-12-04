#ifndef __RF_VOL_JUDGE_H_
#define __RF_VOL_JUDGE_H_

#include "stm32f0xx.h"

//#define RF_STRUCT_INFOR_ADDR   0X0800F800    //PAGE62
//#define RF_STRUCT_INFOR_PAGE   62    //PAGE62

//#define RF_AVERAGE_VOL_ADDR    0X0800FC00    //PAGE63
//#define RF_AVERAGE_VOL_PAGE    0X0800FC00    //PAGE63


#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_VOL_PAGE			63


typedef struct
{
	uint32_t baseaddr;
	uint16_t offer;
	uint16_t average_vol;
}RF_vol_infor_t;

void EreaseAddrPage(uint32_t addr);
uint8_t WriteVolToFlash(uint32_t addr, uint32_t vol);
uint32_t GetAverageVol(uint32_t addr);




#endif


