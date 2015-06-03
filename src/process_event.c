#include "process_event.h"
#include "Link_list.h"
#include "string.h"
#include "event.h"
#include "led_blink.h"
#include "74HC595.h"
#include "seg_led.h"
#include "time.h"
#include "index.h"
#include "button_key.h"

#define SLEEP_TIMEOUT 5000/2   /* 定时器计时周期为 2ms */

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_INIT,&lock_infor,0,0,0};
struct node_struct_t process_event_scan_node;



static void process_event(void);
uint32_t SleepTime_End = 0; 



void Process_Event_Task_Register(void)
{
		lock_operate.lock_state = LOCK_INIT;
		lock_operate.user_num = 0;
		lock_operate.admin_num = 0;
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
		{
			SleepTime_End = time + SLEEP_TIMEOUT;
			if((e.event=BUTTON_KEY_EVENT) || (e.event==RFID_CARD_EVENT))
					Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
			else
			{
					Hal_LED_Display_Set(HAL_LED_MODE_ON, Random16bitdata());
			}
		}
				
		switch(lock_operate.lock_state)
		{
			case LOCK_INIT:
			{
					if(Get_id_Number()!=0)
					{
						 SegDisplayCode = LEDDisplayCode[16];
						 SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[16];/* display -- */
					}
					else
					{
						 SegDisplayCode = LEDDisplayCode[19];   /* u */
						 SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[20]; /*  displya u n*/
					}
					lock_operate.lock_state = LOCK_IDLE;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//

				
				break;
			}
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
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
					//HAL_LED_Blue_ON_Contine(LED_BLUE_ALL_ON_VALUE, 5000);
				}

				break;
			case LOCK_READY:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Lock_EnterIdle();
							lock_operate.lock_state = LOCK_IDLE;
							break;
						case KEY_DEL_SHORT:
							lock_operate.lock_action = DELETE_ID;
							lock_operate.lock_state = DELETE_USER_BY_FP;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								SegDisplayCode = LEDDisplayCode[18];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[15];/* FP */
							}
							else
							{
								SegDisplayCode = LEDDisplayCode[13];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[10];/*  AD */
							}
						case KEY_ADD_SHORT:
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_Null_ID(0);
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								SegDisplayCode = LEDDisplayCode[lock_operate.id%10];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[lock_operate.id/10];
								//Hal_SEG_LED_Display_Set(HAL_LED_MODE_BLINK, SegDisplayCode );
							}	
							else
							{
								SegDisplayCode = LEDDisplayCode[13];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[10];/* AD */
								lock_operate.lock_state = WAIT_AUTHENTIC;
							}
							lock_operate.lock_action = ADD_ID;
							break;
						case KEY_DEL_LONG:
							break;
						case KEY_ADD_LONG:
							break;
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_Null_ID(0);
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								SegDisplayCode = LEDDisplayCode[lock_operate.id%10];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[lock_operate.id/10];
								//Hal_SEG_LED_Display_Set(HAL_LED_MODE_BLINK, SegDisplayCode );
							}	
							else
							{
								SegDisplayCode = LEDDisplayCode[13];
								SegDisplayCode = (SegDisplayCode<<8) | LEDDisplayCode[10];/* AD */
								lock_operate.lock_state = WAIT_AUTHENTIC;
							}
							lock_operate.lock_action = ADD_ID;
							
							break;
						case KEY_INIT_LONG:
							break;
						default:
							break;
					}
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_BLINK, SegDisplayCode );
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					
				}
					break;
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







