#ifndef __MOTOR_H_
#define __MOTOR_H_
#include "stm32f0xx.h"

#define LOCK_MODE_FLAG_ADDR  					0X0800E800   // page58
#define LOCK_MODE_FLAG						0x5588  //����


typedef struct
{
	uint8_t door_mode;   //00������ģʽ��01������ģʽ
	uint8_t door_state;  //00:�رգ�01����
}Door_Infor_t;

typedef enum
{
	MOTOR_NONE,
	MOTOR_FORWARDK,
	MOTOR_STOP,
	MOTOR_REVERSE
}Motor_State_t;

void Motor_Drive_Forward(void);
void Motor_Drive_Stop(void);
void Motor_Drive_Reverse(void);


void Motor_GPIO_Init(void);

uint16_t Get_Open_Normal_Motor_Flag(void);
void Write_Open_Normally_Flag(void);
void Erase_Open_Normally_Mode(void);





#endif
