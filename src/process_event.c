#include "process_event.h"
#include "Link_list.h"
#include "string.h"
#include "event.h"
#include "led_blink.h"
#include "74HC595.h"
#include "seg_led.h"
#include "time.h"
#include "index.h"

#define SLEEP_TIMEOUT 5000/2   /* 定时器计时周期为 2ms */

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_READY};
struct node_struct_t process_event_scan_node;



static void process_event(void);
uint32_t SleepTime_End = 0; 



void Process_Event_Task_Register(void)
{
		lock_operate.lock_state = LOCK_IDLE;
		lklt_insert(&process_event_scan_node, process_event, NULL, 10/5);
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
}

void Lock_EnterIdle(void)
{
		lock_operate.lock_state = LOCK_IDLE;
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);//turn off SEG8_LED
		Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);  //turn off all led
}

static void process_event(void)
{
	uint32_t time;
	uint16_t SegDisplayCode;
	Hal_EventTypedef e; 
	
		time= GetSystemTime();
	
		if((lock_operate.lock_state!=LOCK_IDLE)&&(time >= SleepTime_End))
			Lock_EnterIdle();
		
		e = USBH_GetEvent();
	  if(e.event==EVENT_NONE)
			return;
		else
			SleepTime_End = time + SLEEP_TIMEOUT;
				
		switch(lock_operate.lock_state)
		{
			case 	LOCK_IDLE:
				if((e.event==BUTTON_KEY_EVENT) || (e.event==TOUCH_KEY_EVENT))
				{
					if(Get_id_Number()!=0)
					{
						 SegDisplayCode = LEDDisplayCode[16];
						 SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[16];
					}
					else
					{
						 SegDisplayCode = LEDDisplayCode[19];
						 SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[20];
					}

					
					lock_operate.lock_state = LOCK_READY;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//显示--或者u n
					Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
					//HAL_LED_Blue_ON_Contine(LED_BLUE_ALL_ON_VALUE, 5000);
				}

				break;
			case LOCK_READY:                                             
			case WAIT_SELECT_USER_ID:
			case WATI_SELECT_ADMIN_ID:
			case WAIT_PASSWORD_ONE:
			case WATI_PASSWORD_TWO:
			case WAIT_AUTHENTIC:
			case DELETE_USER_BY_FP:
			case DELETE_USER_ALL:
			case DELETE_USER_ID:
			case DELETE_ADMIN_BY_FP:
			case DELETE_ADMIN_ALL:
			case DELETE_ADMIN_ID:
			case ADD_ID_OK:
			case DELETE_ID_OK:
				break;
			default :
				break;
	}
}







