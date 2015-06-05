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
#include "pwm.h"

#define SLEEP_TIMEOUT 5000/2   /* 定时器计时周期为 2ms */

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_INIT,&lock_infor,0,0,0};
struct node_struct_t process_event_scan_node;



static void process_event(void);
uint32_t SleepTime_End = 0; 

static uint16_t GetDisplayCodeNull(void);
static uint16_t GetDisplayCodeAD(void);
static uint16_t GetDisplayCodeFP(void);
static uint16_t GetDisplayCodeActive(void);
static uint16_t GetDisplayCodeNum(uint8_t num);

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
			if((e.event==BUTTON_KEY_EVENT) || (e.event==RFID_CARD_EVENT))
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
						 SegDisplayCode = GetDisplayCodeActive();
						
					}
					else
					{
						 SegDisplayCode = GetDisplayCodeNull();   
						 Hal_Beep_Blink (2, 50, 50);  //需要看效果
					}
					lock_operate.lock_state = LOCK_READY;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
			}
			break;
			
			case 	LOCK_IDLE:
				if((e.event==BUTTON_KEY_EVENT) || (e.event==TOUCH_KEY_EVENT))
				{
					if(Get_id_Number()!=0)
					{
						SegDisplayCode = GetDisplayCodeActive();
					}
					else
					{
						SegDisplayCode = GetDisplayCodeNull();  
						Hal_Beep_Blink (2, 50, 50);  //需要看效果
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
							if(Get_User_id_Number()>0)
							{
								lock_operate.lock_action = DELETE_ID;
								if(lock_operate.plock_infor->work_mode==NORMAL)
								{
									lock_operate.lock_state = DELETE_USER_BY_FP;
									SegDisplayCode = GetDisplayCodeFP();
								}
								else
								{
									lock_operate.lock_state = WAIT_AUTHENTIC;
									SegDisplayCode = GetDisplayCodeAD();
								}
							}
							else
							{
								SegDisplayCode = GetDisplayCodeNull();   /* un */
								Hal_Beep_Blink (2, 50, 50);  //需要看效果
								lock_operate.lock_state = LOCK_READY;  
							}
							break;
							
						case KEY_ADD_SHORT:
							lock_operate.lock_action = ADD_ID;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_Null_ID(0);  //数据已满，没有处理
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.lock_state = WAIT_AUTHENTIC;
							}						
							break;
						case KEY_DEL_LONG:
							if(Get_Admin_id_Number()!=0)
							{
								lock_operate.plock_infor->work_mode=SECURITY;
								lock_operate.lock_action = DELETE_ID;
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
							}
							else /* 不确定显示什么*/
							{
								lock_operate.lock_state = LOCK_READY;
								SegDisplayCode = GetDisplayCodeNull();   /* un */
								Hal_Beep_Blink (2, 50, 50);  //需要看效果
							}
							
							break;
							
						case KEY_ADD_LONG:
							lock_operate.lock_action = ADD_ID;
							if(lock_operate.plock_infor->work_mode==SECURITY)
							{
								lock_operate.id = Find_Next_Null_ID(95);
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
							}
							else
							{
								lock_operate.id = Find_Next_Null_ID(95);
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							}
							break;		
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							lock_operate.lock_action = ADD_ID;	
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_Null_ID(0);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.id = 0;
								lock_operate.lock_state = WAIT_AUTHENTIC;
							}		
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
				if(e.event==BUTTON_KEY_EVENT) 
				{
					
				}
				
				break;
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


static uint16_t GetDisplayCodeNull(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[19];   /* u */
	code = (code<<8) | LEDDisplayCode[20]; /*  displya u n*/				
	return code;
}

static uint16_t GetDisplayCodeAD(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[13];
	code = (code<<8) | LEDDisplayCode[10];/*  AD */
	return code;
}
static uint16_t GetDisplayCodeFP(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[18];
	code = (code<<8) | LEDDisplayCode[15];/*  FP */
	return code;	
}

static uint16_t GetDisplayCodeActive(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[16];
	code = (code<<8) | LEDDisplayCode[16];/*  FP */
	return code;	
}

static uint16_t GetDisplayCodeNum(uint8_t num) /* Number */
{
	uint16_t code;
	
	if(num>99)
		return 0;
	
	code = LEDDisplayCode[num%10];
	code = (code<<8) | LEDDisplayCode[num/10];	
	return code;	
}

