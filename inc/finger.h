#ifndef _FINGER_H_
#define _FINGER_H_
#include "stm32f0xx.h"


#define ACK_SUCCESS      0x00 
#define ACK_FAIL     	 0x01 
#define ACK_FULL       	 0x04  
#define ACK_NOUSER     	 0x05  
#define ACK_USER_EXIST   0x07  
#define ACK_TIMEOUT  	 0x08 


#define MATCH_CMD 	0X0D
#define REGIST1_CMD 0X01
#define REGIST2_CMD 0X02
#define REGIST3_CMD 0X03


#define FINGER 	1

void finger_init(void);
void Finger_Regist_CMD2(void);//两次按压指纹注册
void Finger_Regist_CMD3(void);
uint16_t Get_Finger_Num(uint16_t *num);
void Match_finger(void);
void Exit_Finger_Current_Operate(void);
uint8_t Delelte_ONE_Finger(uint16_t id);
uint8_t Delete_All_Finger(void);
#endif
