#ifdef __DEBUG_H_
#define __DEBUG_H_
#include "stdio.h"

const char* lock_state_str[];
char* Get_button_to_str(uint8_t key);

#ifdef _DEBUG_
	#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
	#define DEBUG(format, ...)
#endif

/*
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
	DELETE_USER_BY_FP,
	DELETE_USER_ALL,
	DELETE_USER_ID,
	DELETE_ADMIN_BY_FP,
	DELETE_ADMIN_ALL,
	DELETE_ADMIN_ID,
	ADD_ID_OK,
	DELETE_ID_OK
	
}LOCK_STATE;
*/




#endif


