#ifndef _FINGER_H_
#define _FINGER_H_
#include "stm32f0xx.h"


#define ACK_SUCCESS      0x00 //执行成功
#define ACK_FAIL     	 0x01 //执行失败
#define ACK_FULL       	 0x04 //数据库满
#define ACK_NOUSER     	 0x05  //没有这个用户
#define ACK_USER_EXIST   0x07  //用户已存在
#define ACK_TIMEOUT  	 0x08  //图像采集超时
#define ACK_BREAK   	 0x18  //终止当前指令
#define ACK_IMAGEFAIL 	 0x84   	//图像不合格


#define MATCH_CMD 	0X0C
#define RES_MATCH_SUCESS_CMD	0x01
#define RES_MATCH_TIMEOUT_CMD	0x08
#define RES_MATCH_FAIL_CMD		0x00

#define REGIST1_CMD 0X01
#define REGIST2_CMD 0X02
#define REGIST3_CMD 0X03


//#define FINGER 	1


typedef enum
{
	FP_INIT,
	FP_IDLY,
	FP_REGISTING,
	FP_ID_DELETEING,
	FP_ALL_DELETEING,
	FP_GET_NUM,
	FP_1_N_MATCH,
}finger_state_t;

extern finger_state_t finger_state;
extern uint8_t is_finger_ok; 

void finger_init(void);
void Finger_Regist_CMD1(void);
void Finger_Regist_CMD2(void);//两次按压指纹注册
void Finger_Regist_CMD3(void);
void Exit_Finger_Current_Operate(void);
uint16_t Get_Finger_Num(uint16_t *num);
uint8_t Delelte_ONE_Finger(uint16_t id);
uint8_t Delete_All_Finger(void);
uint8_t Finger_Set_DenyingSame(void);
void Match_finger(void);
void Finger_Scan(void);
void Finger_LDO_Init(void);
void Finger_RF_LDO_Disable(void);
void Finger_RF_LDO_Enable(void);
void finger_wakeup_detect_pin_init(void);
uint8_t is_finger_wakeup(void);
//int8_t Get_Finger_From_InterIndex(uint16_t d);
#endif
