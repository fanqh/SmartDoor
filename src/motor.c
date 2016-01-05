#include "motor.h"
#include "process_event.h"
#include "led_blink.h"

#define MOTOR_PINA   GPIO_Pin_11
#define MOTOR_PINB   GPIO_Pin_10
#define MOTOR_PORT   GPIOB

Door_Infor_t door_infor;	
/*
	void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
*/	
	
void Motor_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	//PB10 PB11   
	GPIO_InitStructure.GPIO_Pin = MOTOR_PINA | MOTOR_PINB;		           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(MOTOR_PORT, &GPIO_InitStructure);
	Motor_Drive_Stop();
	
}

//void Motor_Init(void)
//{
//	uint8_t =*((Door_Infor_t*)DOOR_INFOR_ADDR);
//	if((door_infor.door_mode == 0xff) || (door_infor.door_state==0xff))
//	{
//		door_infor.door_mode = 0;//正常模式
//		door_infor.door_state =0;//关闭状态
//		lock_operate.lock_state = LOCK_CLOSE;
////		Led_Open_Normally_OFF();
//		Save_DoorInfor(&door_infor);
//	}	
//}





void Motor_Drive_Reverse(void)
{
	 GPIO_SetBits(MOTOR_PORT, MOTOR_PINA | MOTOR_PINB);
	 GPIO_ResetBits(MOTOR_PORT, MOTOR_PINB);
}
void Motor_Drive_Stop(void)
{
	GPIO_SetBits(MOTOR_PORT,MOTOR_PINA | MOTOR_PINB);
}

void Motor_Drive_Forward(void)
{
   GPIO_SetBits(MOTOR_PORT, MOTOR_PINA | MOTOR_PINB);
   GPIO_ResetBits(MOTOR_PORT, MOTOR_PINA);
}



uint16_t Get_Open_Normal_Motor_Flag(void)
{
	return *((uint16_t*)LOCK_MODE_FLAG_ADDR);
}

void Write_Open_Normally_Flag(void)
{
	FLASH_Unlock();
	FLASH_ErasePage(LOCK_MODE_FLAG_ADDR);
	FLASH_ProgramHalfWord((uint32_t)LOCK_MODE_FLAG_ADDR, LOCK_MODE_FLAG);
	FLASH_Lock();
}

void Erase_Open_Normally_Mode(void)
{
	FLASH_Unlock();
	FLASH_ErasePage(LOCK_MODE_FLAG_ADDR);
	FLASH_Lock();
}







