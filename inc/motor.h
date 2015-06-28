#ifndef __MOTOR_H_
#define __MOTOR_H_
#include "stm32f0xx.h"

#define DOOR_INFOR_ADDR  					0X0800E800


typedef struct
{
	uint8_t door_mode;   //00：正常模式，01：常开模式
	uint8_t door_state;  //00:关闭，01：打开
}Door_Infor_t;

typedef enum
{
	MOTOR_NONE,
	MOTOR_FORWARDK,
	MOTOR_STOP,
	MOTOR_REVERSE
}Motor_State_t;

extern Door_Infor_t door_infor;	
void Motor_Drive_Forward(void);
void Motor_Drive_Stop(void);
void Motor_Drive_Reverse(void);
void Save_DoorInfor(Door_Infor_t *infor);

void Motor_GPIO_Init(void);
void Motor_Init(void);





#endif
