#include "lock_key.h"
#include "rf_vol_judge.h"
#include "delay.h"

#define 	KEY_LOCK_PIN_PORT 	GPIOA
#define 	KEY_LOCK_PIN  		GPIO_Pin_8
#define   	KEY_LOCK_PIN_CLK    RCC_AHBPeriph_GPIOA



void Key_Lock_Pin_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHBPeriphClockCmd(KEY_LOCK_PIN_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Pin = KEY_LOCK_PIN;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		        
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(KEY_LOCK_PIN_PORT, &GPIO_InitStruct);
}

uint8_t Get_Lock_Pin_State(void)
{
	uint8_t state1, state2;
	state1 = GPIO_ReadInputDataBit( KEY_LOCK_PIN_PORT,KEY_LOCK_PIN);
	state2 =GPIO_ReadInputDataBit( KEY_LOCK_PIN_PORT,KEY_LOCK_PIN);
	if(state1== state2)
		return state1;
	else
		return 1;
}

uint32_t GetLockFlag(uint32_t addr)
{
	uint32_t vol;
	
	vol = *(uint32_t*)addr;
	return vol;
}

uint8_t WriteLockFlag(uint32_t addr, uint32_t flag)
{
	FLASH_Status ret;
	
	FLASH_Unlock();
	if(GetLockFlag(addr)!=0xffffffff)
	{
		printf("W: erease...\r\n");
		EreaseAddrPage(addr);
	}
	printf(" %X ,write %d to flash %X\r\n",GetLockFlag(addr), flag,addr);
	ret = FLASH_ProgramWord(addr, flag);
	delay_ms(1);
	printf("W: %X,ret = %d\r\n",GetLockFlag(addr),ret);
	FLASH_Lock(); 	
	if(ret!=FLASH_COMPLETE)
		return 0;
	else
		return 1;
}

