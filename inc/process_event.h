#ifndef __PROCESS_EVENT_H_
#define __PROCESS_EVENT_H_
#include "stm32f0xx.h"
#include "index.h"



typedef enum
{
	LOCK_INIT =0,
	LOCK_IDLE ,
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
	DELETE_ID_OK
	
}LOCK_STATE;

typedef enum
{
	ACTION_NONE=0,
	ADD_USER,
	DELETE_USER,
	ADD_ADMIN,
	DELETE_ADMIN
	
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
} lock_operate_srtuct_t;

void Process_Event_Task_Register(void);

#endif
