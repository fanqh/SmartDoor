#ifndef __PROCESS_EVENT_H_
#define __PROCESS_EVENT_H_
#include "stm32f0xx.h"
#include "index.h"
#include "motor.h"




#define SLEEP_TIMEOUT 5000/2  			  /* 定时器计时周期为 2ms */
#define Beep_Null_Warm()							{Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 1, 200, 200);}  //id空报警
#define Beep_Touch_Tone()							{Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}  //touch 长度到提示
#define Beep_Fail_Warm()      				Hal_Beep_Blink (1, 50, 50) 
#define Beep_Delete_All_Warm()				{Hal_Beep_Blink (4, 100, 100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Beep_Delete_ID_Tone()					{Hal_Beep_Blink (2, 100, 100);Hal_LED_Blink (LED_BLUE_ALL_ON_VALUE, 2, 200, 200);}
#define Beep_Register_Fail_Warm() 		{Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Beep_Register_Sucess_Tone()   {Hal_Beep_Blink (1, 80,30);Hal_LED_Blink (LED_GREEN_ON_VALUE, 20, 100, 100);}
#define Beep_Compare_Fail_Warm()			Hal_Beep_Blink (2, 50,50)
#define Flash_ID_Full_Warm()					{Hal_Beep_Blink (2, 100, 100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Flash_Comare_Sucess_Warm()			 {Hal_Beep_Blink (2, 80,30);;Hal_LED_Blink (LED_GREEN_ON_VALUE, 3, 200, 200);}
#define Error_ID_Warm()								{Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Comare_Fail_Warm()            {Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 5, 200, 200);}
#define LED_Blink_Compare_Fail_Warm()   Hal_LED_Blink (LED_RED_ON_VALUE, 2, 200, 200)  

uint16_t GetDisplayCodeNull(void);
uint16_t GetDisplayCodeActive(void);

typedef enum
{
	LOCK_INIT =0,
	LOCK_IDLE ,
	LOCK_ACTIVING,
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
	LOCK_CLOSE
	
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

extern void SytemWakeup(void);
uint16_t Lock_EnterIdle(void);
uint16_t Lock_EnterIdle1(void);

#endif
