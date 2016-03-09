#ifndef __PROCESS_EVENT_H_
#define __PROCESS_EVENT_H_
#include "stm32f0xx.h"
#include "index.h"
#include "motor.h"
#include "led_blink.h"
#include "event.h"
#include "finger.h"



#define SLEEP_TIMEOUT 7000 			     /* 定时器计时周期为 2ms */

#define PASSWD_SUCESS_ON()      {HC595_ShiftOut16(SER_LED_INTERFACE,LED_GREEN_ON_VALUE); HalLedControl.mode = HAL_LED_MODE_OFF; Music_PWM();}   
/*
1. WATI_SELECT_ADMIN_ID下按KEY_DEL_SHORT  adminID已满时


*/

#define Beep_Null_Warm()					{Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 1, 200, 200);}  //id空报警  错误状态
#define LOCK_ERR_Warm()						{Hal_Beep_Blink (0, 200,200);Hal_LED_Blink (LED_RED_ON_VALUE, 0, 200, 200);}  //密码输入错误次数超过3次报警
#define LOCK_ENTER_NOMAL_MODE_WARM()		PSWD_Less_TwoBeep()  //进入常开模式提示
#define Beep_PSWD_ONE_OK_Warm() 		  	  {PSWD_Less_TwoBeep();}
//190ms on, 1.25KHZ
#define PASSWD_COMPARE_ERR()				{BIT_MORE_TWO_WARM();Hal_LED_Red_Blink_Once();}//{Hal_Beep_Blink (2, 50,50); Hal_LED_Red_Blink_Once();}  //密码比对错误提示
//909HZ(1.1mz,600us),833HZ(1.2ms,500us),1.35KHZ(740us,360us)  440ms
//#define PASSWD_TWO_OK()						{Music_PWM();Hal_LED_Blink_GREEN_ONCE();}  
#define PASSWD_COMPARE_OK()					{Music_PWM();Hal_LED_Blink_GREEN_ONCE();} //密码比对成功
											 //{HC595_ShiftOut16(SER_LED_INTERFACE,LED_GREEN_ON_VALUE); Music_PWM();\  
											 //HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);delay_ms(240);\
											 //HC595_ShiftOut16(SER_LED_INTERFACE,LED_BLUE_ALL_ON_VALUE);}
#define BIT_MORE_TWO_WARM()					 {Beep_Bit_More();} //ID错误 开机提示音
#define Beep_Register_Fail_Warm() 		 	 {Regist_Fail_TwoBeep();Hal_LED_Blink_Two_Blue_ON_Block();}  //密码注册错误
#define PASSWD_ID_OVER_Indication()			 BIT_MORE_TWO_WARM()	
#define PASSWD_Delete_ALL_ID()				 {Beep_Four_Time();}  //HC595_ShiftOut16(SER_LED_INTERFACE, LED_GREEN_ON_VALUE);


#define ERR_UNKNOWN()   					{PASSWD_COMPARE_ERR();printf("system err????\r\n");}





uint16_t GetDisplayCodeNull(void);
uint16_t GetDisplayCodeActive(void);

typedef enum
{
	LOCK_INIT =0,
	LOCK_IDLE ,             
	LOCK_UNLOCK_WARM,
	LOCK_READY,                   //1
	WAIT_SELECT_USER_ID,
	WATI_SELECT_ADMIN_ID,
	WAIT_PASSWORD_ONE,           //1
	WATI_PASSWORD_TWO,           //1
	WAIT_AUTHENTIC,              //1
	WAIT_SELECT_DELETE_MODE,
//	WAIT_DELETE_ADMIN_MODE,
	DELETE_USER_BY_FP,              //1
	DELETE_USER_ALL,             
 	DELETE_USER_ID,
	DELETE_ADMIN_BY_FP,             //1
	DELETE_ADMIN_ALL,
	DELETE_ADMIN_ID,
	ADD_ID_OK,
	DELETE_ID_OK,
	LOCK_OPEN_CLOSE,
	LOCK_OPEN,
	LOCK_CLOSE,
	LOCK_ERR,
	LOCK_OPEN_NORMAL,
	LOCK_GET_ID_NUM
	
}LOCK_STATE;

typedef enum
{
	ACTION_NONE=0,
	ADD_USER,
	DELETE_USER,
	ADD_ADMIN,
	DELETE_ADMIN,
	DELETE_ALL,
	GET_ID_NUM
	
}LOCK_ACTION;




typedef struct
{
	LOCK_ACTION lock_action;
	LOCK_STATE lock_state;
	lock_infor_t *plock_infor;
	uint8_t id;
	uint8_t user_num;
	uint8_t  admin_num;
	uint16_t BatVol;
	finger_state_t *fp_state;
//	Door_Infor_t *pDooInfor;
} lock_operate_srtuct_t;

void Process_Event_Task_Register(void);

extern lock_operate_srtuct_t lock_operate;
extern Hal_EventTypedef gEventOne;

extern void SytemWakeup(void);
uint16_t Lock_EnterIdle(void);
uint16_t Lock_EnterIdle1(void);
uint8_t is_Motor_Moving(void);
void process_event(void);
uint16_t GetDisplayCodeBatteryLowlMode(void);
uint16_t Lock_EnterReady(void);

#endif
