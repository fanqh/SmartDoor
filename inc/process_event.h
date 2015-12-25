#ifndef __PROCESS_EVENT_H_
#define __PROCESS_EVENT_H_
#include "stm32f0xx.h"
#include "index.h"
#include "motor.h"
#include "led_blink.h"
#include "event.h"



#define SLEEP_TIMEOUT 5000/2  			     /* 定时器计时周期为 2ms */
#define Beep_Null_Warm()							  {Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 1, 200, 200);}  //id空报警
#define Beep_Touch_Tone()							  {Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}  //touch 长度到提示
#define Beep_Fail_Warm()      				  Hal_Beep_Blink (1, 50, 50) 
#define Beep_Delete_All_Warm()				  {Hal_Beep_Blink (4, 100, 100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Beep_Delete_ID_Tone()					  {Hal_Beep_Blink (2, 100, 100);Hal_LED_Blink (LED_BLUE_ALL_ON_VALUE, 2, 200, 200);}
#define Beep_Register_Sucess_Tone()     {Hal_Beep_Blink (2, 80,30);Hal_LED_Blink (LED_GREEN_ON_VALUE, 20, 100, 100);}
#define Beep_Compare_Fail_Warm()			  {Hal_Beep_Blink (2, 50,50);}
#define Flash_ID_Full_Warm()					  {Hal_Beep_Blink (2, 100, 100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Flash_Comare_Sucess_Warm()	    {Hal_Beep_Blink (2, 80,30);;Hal_LED_Blink (LED_GREEN_ON_VALUE, 3, 200, 200);}
#define LED_Blink_Compare_Fail_Warm()   Hal_LED_Blink (LED_RED_ON_VALUE, 2, 200, 200)  
#define LOCK_ERR_Warm()									{Hal_Beep_Blink (0, 200,200);Hal_LED_Blink (LED_RED_ON_VALUE, 0, 200, 200);}
#define Comare_Fail_Warm()           	  {Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 5, 200, 200);}
#define Battery_Low_Warm()				  {Hal_Beep_Blink (3, 200,200);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}

#define Beep_Register_Fail_Warm() 		  {Regist_Fail_TwoBeep();Hal_LED_Blink (LED_RED_ON_VALUE, 3, 100, 100);}
#define Beep_PSWD_LESS_Warm() 		  	  {PSWD_Less_TwoBeep();Hal_LED_Blink (LED_RED_ON_VALUE, 3, 100, 100);}
//#define Beep_Register_Fail_Warm() 		  {Hal_Beep_Blink (2, 100,100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 100, 100);}
#define Error_ID_Warm()								  {Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}


//40ms on  40ms off, 40ms on  2.38khz
#define PASSWD_ONE_OK()                 {	Beep_PWM_TimeBase_config(500);Beep_PWM_config(230);	Hal_Beep_Blink (2, 50,30);}
//190ms on, 1.25KHZ
#define PASSWD_COMPARE_ERR()						{Beep_Compare_ID_Err(); Hal_LED_Red_Blink_Once();}
//909HZ(1.1mz,600us),833HZ(1.2ms,500us),1.35KHZ(740us,360us)  440ms
#define PASSWD_TWO_OK()									{Music_PWM();Hal_LED_Blink_GREEN_ONCE();}                  //////////////////BEEP 声音不对 注册成功
#define PASSWD_COMPARE_OK()						  {HC595_ShiftOut16(SER_LED_INTERFACE,LED_GREEN_ON_VALUE); Music_PWM();HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);delay_ms(240);HC595_ShiftOut16(SER_LED_INTERFACE,LED_BLUE_ALL_ON_VALUE);}  
#define PASSWD_Delete_ONE_ID()					{HC595_ShiftOut16(SER_LED_INTERFACE,LED_GREEN_ON_VALUE);Beep_Two_Time();HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);delay_ms(240);HC595_ShiftOut16(SER_LED_INTERFACE,LED_BLUE_ALL_ON_VALUE);}      ///蜂鸣器声音和上面的一样
//1.32KHZ(760us,360us),234ms on
#define BIT_MORE_TWO_WARM()							{Beep_Bit_More();}  
#define PASSWD_FULL_Indication()        {HC595_ShiftOut16(SER_LED_INTERFACE, LED_GREEN_ON_VALUE); Beep_Three_Time();}
#define PASSWD_ID_OVER_Indication()			BIT_MORE_TWO_WARM()	
#define Touch_Once__Warm()							BIT_MORE_TWO_WARM()	//{Hal_Beep_Blink (1, 1,50);Hal_LED_Blink_RED_ONCE();}

#define PASSWD_Delete_ALL_ID()					{Beep_Four_Time();}  //HC595_ShiftOut16(SER_LED_INTERFACE, LED_GREEN_ON_VALUE);


#define PASSWD_SUCESS_ON()      {HC595_ShiftOut16(SER_LED_INTERFACE,LED_GREEN_ON_VALUE); HalLedControl.mode = HAL_LED_MODE_OFF; Music_PWM();}   
/*
1. WATI_SELECT_ADMIN_ID下按KEY_DEL_SHORT  adminID已满时


*/





uint16_t GetDisplayCodeNull(void);
uint16_t GetDisplayCodeActive(void);

typedef enum
{
	LOCK_INIT =0,
	LOCK_IDLE ,
	LOCK_UNLOCK_WARM,
	LOCK_READY,
	WAIT_SELECT_USER_ID,
	WATI_SELECT_ADMIN_ID,
	WAIT_PASSWORD_ONE,
	WATI_PASSWORD_TWO,
	WAIT_AUTHENTIC,
	WAIT_SELECT_DELETE_MODE,
//	WAIT_DELETE_ADMIN_MODE,
	DELETE_USER_BY_FP,
	DELETE_USER_ALL,
	DELETE_USER_ID,
	DELETE_ADMIN_BY_FP,
	DELETE_ADMIN_ALL,
	DELETE_ADMIN_ID,
	ADD_ID_OK,
	DELETE_ID_OK,
	LOCK_OPEN_CLOSE,
	LOCK_OPEN,
	LOCK_CLOSE,
	LOCK_ERR
	
}LOCK_STATE;

typedef enum
{
	ACTION_NONE=0,
	ADD_USER,
	DELETE_USER,
	ADD_ADMIN,
	DELETE_ADMIN,
	DELETE_ALL
	
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
	Door_Infor_t *pDooInfor;
} lock_operate_srtuct_t;

extern uint8_t WakeupFlag;

void Process_Event_Task_Register(void);

extern lock_operate_srtuct_t lock_operate;
extern Hal_EventTypedef gEventOne;

extern void SytemWakeup(void);
uint16_t Lock_EnterIdle(void);
uint16_t Lock_EnterIdle1(void);
uint8_t is_Motor_Moving(void);

#endif
