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
#include "mpr121_key.h"
#include "debug.h"
#include "string.h"
//#include "motor.h"
#include "delay.h"
#include "main.h"
#include "sleep_mode.h"
#include "RF.h"



#define DEBUG_  0

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_READY,&lock_infor,0,0,0,0xffff,&door_infor};
struct node_struct_t process_event_scan_node;
static uint32_t MotorEndTime = 0;
static uint32_t SleepTime_End = 0; 
static uint32_t Lock_Restrict_Time=0;	
static uint32_t PW_Err_Count = 0;
static uint8_t Touch_Clear = 0;  /* 1: 已做清除操作，0：没做 */
char gpswdOne[TOUCH_KEY_PSWD_LEN+1];
static Hal_EventTypedef gEventOne;
static LOCK_STATE Delete_Mode_Temp = DELETE_USER_BY_FP;
static uint8_t gOperateBit = 0;   //0  digits  1 decimal  2 warm 
Motor_State_t motor_state = MOTOR_NONE;

uint8_t WakeupFlag = 0; // 0激活事件已处理，0x01: 按键事件，0x02: 定时器


static uint16_t GetDisplayCodeAD(void);
static uint16_t GetDisplayCodeFP(void);
static uint16_t GetDisplayCodeNum(uint8_t num);
static uint16_t GetDisplayCodeFU(void);
static uint16_t GetDisplayCodeAL(void);
static uint16_t GetDisplayCodeFE(void);

static void process_event(void);

static const char* lock_state_str[]=
{	"INIT",
	"IDLE" ,
	"READY",
	"WAIT_SELECT_USER_ID",
	"WATI_SELECT_ADMIN_ID",
	"WAIT_PASSWORD_ONE",
	"WATI_PASSWORD_TWO",
	"WAIT_AUTHENTIC",
	"DELETE_USER_BY_FP",
	"DELETE_USER_ALL",
	"DELETE_USER_ID",
	"DELETE_ADMIN_BY_FP",
	"DELETE_ADMIN_ALL",
	"DELETE_ADMIN_ID",
	"ADD_ID_OK",
	"DELETE_ID_OK"
};

static const char* event_str[]=
{
	"EVENT_NONE" ,
	"BUTTON_KEY",
	"TOUCH_KEY",
	"RFID_CARD"
};

char Button_str[32];
void Get_button_to_str(uint8_t key)
{

	switch(key)
	{
		case 	KEY_CANCEL_SHORT :
			strcpy(Button_str, "KEY_CANCEL_SHORT");
			break;
		case 	KEY_DEL_SHORT 		 :
						strcpy(Button_str, "KEY_DEL_SHORT");
			break;
		case 	KEY_OK_SHORT  		 :
						strcpy(Button_str, "KEY_OK_SHORT");
			break;
		case 	KEY_INIT_SHORT 		 :
						strcpy(Button_str, "KEY_INIT_SHORT");
			break;
		case 	KEY_ADD_SHORT   	:
						strcpy(Button_str, "KEY_ADD_SHORT");
			break;

		case 	KEY_CANCEL_LONG	 :
						strcpy(Button_str, "KEY_CANCEL_LONG");
			break;
		case 	KEY_DEL_LONG 		 :
						strcpy(Button_str, "KEY_DEL_LONG");
			break;
		case 	KEY_OK_LONG  		 :
						strcpy(Button_str, "KEY_OK_LONG");
			break;
		case 	KEY_INIT_LONG 	 :
					strcpy(Button_str, "KEY_INIT_LONG");
			break;
		case 	KEY_ADD_LONG   	 :
			strcpy(Button_str, "KEY_ADD_LONG");
			break;
		default :
			strcpy(Button_str, "unknow_key"); 
	}
}

	
	
	

void Process_Event_Task_Register(void)
{
		lock_operate.lock_state = LOCK_READY;
		lock_operate.user_num = 0;
		lock_operate.admin_num = 0;
		lklt_insert(&process_event_scan_node, process_event, NULL, 25*TRAV_INTERVAL);//TRAV_INTERVAL  10ms
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
}


static uint16_t Lock_EnterReady(void)
{
	uint16_t SegDisplayCode;
	
	gOperateBit =0;
//	if(Get_id_Number()!=0)
	{
		SegDisplayCode = GetDisplayCodeActive();
	}
//	else
//	{
//		SegDisplayCode = GetDisplayCodeNull();  
//		Beep_Null_Warm();
//	}			
	fifo_clear(&touch_key_fifo);
	lock_operate.lock_state = LOCK_READY;
	//Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
//	printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
	
	return SegDisplayCode;
}
static uint16_t Lock_Enter_DELETE_USER_BY_FP(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_USER_BY_FP;
	code = GetDisplayCodeFP();
	return code;
}





static uint16_t Lock_Enter_DELETE_ADMIN_BY_FP(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_ADMIN_BY_FP;
	code = GetDisplayCodeFP();
	return code;
}

static uint16_t Lock_Enter_Wait_Delete_ID(void)
{    
		uint16_t code;
	
//		lock_operate.id = 0;
		if(lock_operate.lock_action==DELETE_ADMIN)
			Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
		else
			Delete_Mode_Temp = DELETE_USER_BY_FP;
		code = GetDisplayCodeFP();
		return code;
}



static void RTC_Config1(void)
{	
		RTC_InitTypeDef   RTC_InitStructure;
		RTC_AlarmTypeDef  RTC_AlarmStructure;
		RTC_TimeTypeDef   RTC_TimeStructure;
	
			/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
			/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);
	    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_InitStructure.RTC_AsynchPrediv = 160;  //160/40k = 4ms
    RTC_InitStructure.RTC_SynchPrediv = 400;    
		RTC_Init(&RTC_InitStructure);		
		
		    
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x00;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x00;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x0;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
    RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure); 
		/* Set the alarm 250ms */
		RTC_AlarmSubSecondConfig(RTC_Alarm_A, 1, RTC_AlarmSubSecondMask_None);  //(PREDIV_S-1)*3ms
    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    /* Enable the alarm */
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
		
				/* Set the time to 01h 00mn 00s AM */
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours   = 0x00;
		RTC_TimeStructure.RTC_Minutes = 0x00;
		RTC_TimeStructure.RTC_Seconds = 0x00;  
		
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);	
}

static void RTC_Config(void)
{	
		RTC_InitTypeDef   RTC_InitStructure;
		RTC_AlarmTypeDef  RTC_AlarmStructure;
		RTC_TimeTypeDef   RTC_TimeStructure;
	
			/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
			/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);
	    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_InitStructure.RTC_AsynchPrediv = 120;  //120/40k = 3ms
    RTC_InitStructure.RTC_SynchPrediv = 300;    
		RTC_Init(&RTC_InitStructure);		
		
		    
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x00;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x00;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x0;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
    RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure); 
		/* Set the alarm 250ms */
		RTC_AlarmSubSecondConfig(RTC_Alarm_A, 1, RTC_AlarmSubSecondMask_None);  //(PREDIV_S-1)*3ms
    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    /* Enable the alarm */
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
		
				/* Set the time to 01h 00mn 00s AM */
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours   = 0x00;
		RTC_TimeStructure.RTC_Minutes = 0x00;
		RTC_TimeStructure.RTC_Seconds = 0x00;  
		
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);	
}
uint16_t Lock_EnterIdle(void)
{
		uint16_t retry = 0;
	
		while(mpr121_get_irq_status()==0)
		{
			delay_us(1);
			retry++;
			if(retry>500)
				return 0;
		}
		mpr121_enter_standby();
		RF_Lowpower_Set();		

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
		/*  Enable the LSI OSC */
		RCC_LSICmd(ENABLE);
		/* Wait till LSI is ready */
		retry = 0;
		while ((RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)&&(retry<100))
		{
			delay_us(1);
			retry++;
		}	
	#if 1
			RTC_Config();
	#endif
		PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE);
		PWR_ClearFlag(PWR_FLAG_WU); 
		if(mpr121_get_irq_status()!=0)
			PWR_EnterSTANDBYMode(); 
		
		 return 0xffff;
}


uint16_t Lock_EnterIdle1(void)
{
		uint16_t retry = 0;
	
		while(mpr121_get_irq_status()==0)
		{
//			delay_us(1);
			retry++;
			if(retry>5000)
				return 0;
		}
		RF_Lowpower_Set();
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
			/*  Enable the LSI OSC */
		RCC_LSICmd(ENABLE);
		/* Wait till LSI is ready */
		retry = 0;
		while ((RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)&&(retry<1000))
		{
			//delay_us(1);
			retry++;
		}	
	#if 1
			RTC_Config();
	#endif
		PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE);
		PWR_ClearFlag(PWR_FLAG_WU); 
		if(mpr121_get_irq_status()!=0)
			PWR_EnterSTANDBYMode(); 
		
		 return 0xffff;
}

void Lock_FU_Indication(void)
{
	uint16_t code;
	code = GetDisplayCodeFU();   /* un */
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code );
	Beep_Three_Time();
	Lock_EnterIdle(); 	
}
void Lock_NULL_Indication(void)
{
		uint16_t code;
		code = GetDisplayCodeNull();   /* un */
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, code );
		Beep_Null_Warm_Block();
		Lock_EnterIdle(); 	
}
void Action_Delete_All_ID(void)
{
	uint16_t SegDisplayCode;
	
	Erase_All_id();
	PASSWD_Delete_ALL_ID();
	delay_ms(80);
	SegDisplayCode = GetDisplayCodeNull(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode ); 
	Beep_Three_Time();
	Lock_EnterIdle();
}
void Lock_Enter_Err(void)
{
	uint16_t SegDisplayCode;
	
	HC595_Power_ON();
	SegDisplayCode = GetDisplayCodeFE();
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
	HC595_ShiftOut16(SER_LED_INTERFACE, LED_RED_ON_VALUE);
	
	Beep_Three_Time();
	HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);
	HC595_Power_OFF();
	Lock_Restrict_Time = GetSystemTime() + 1000*60*3/2;//3min
	lock_operate.lock_state = LOCK_ERR; 
	HalBeepControl.SleepActive =1;	
}


uint8_t is_Motor_Moving(void)
{
	if((lock_operate.lock_state==LOCK_OPEN_CLOSE)||(lock_operate.lock_state==LOCK_OPEN)||(lock_operate.lock_state==LOCK_CLOSE))
		return 1;
	else
		return 0;
}

static void process_event(void)
{
	int8_t id;
	uint32_t time;
	uint16_t SegDisplayCode;
	Hal_EventTypedef e; 
	
		time= GetSystemTime();
		e = USBH_GetEvent();
	
		if((lock_operate.lock_state!=LOCK_IDLE)&&(time >= SleepTime_End)&&(lock_operate.lock_state!=LOCK_ERR)&&(!is_Motor_Moving()))
				Lock_EnterIdle();			
		if((lock_operate.lock_state==LOCK_ERR)&&(time>Lock_Restrict_Time))
				Lock_EnterIdle();
		
	  if((e.event==EVENT_NONE)
			&&(!((lock_operate.lock_state==LOCK_OPEN_CLOSE)||(lock_operate.lock_state==LOCK_OPEN)||(lock_operate.lock_state==LOCK_CLOSE))))//需要替换掉
			return;
		else
		{
			SleepTime_End = time + SLEEP_TIMEOUT;
			if(e.event==TOUCH_KEY_EVENT)
			{
					Hal_LED_Display_Set(HAL_LED_MODE_BLINK, Random16bitdata());
			}
		}
		

#if DEBUG_

		if(e.event==BUTTON_KEY_EVENT)
		{
			printf("-e: %s, Prestate: %s, id=%d", "BUTTON_KEY_EVENT", lock_state_str[lock_operate.lock_state], lock_operate.id);
			printf("\r\n");
		}
		else if(e.event==TOUCH_KEY_EVENT)
		{
			char str[64];
			
			printf("-e: %s, Prestate: %s, Touch_Value: %c" , "TOUCH_KEY_EVENT\r\n", lock_state_str[lock_operate.lock_state], (char)(e.data.KeyValude));
			snprintf(str, Get_fifo_size(&touch_key_fifo)+1, (char*)(touch_key_buf));
			printf("len= %d, touch_fifo: %s\r\n", Get_fifo_size(&touch_key_fifo), str);
		}
		else if(e.event==RFID_CARD_EVENT)
		{
			printf("-e: %s, Prestate: %s\r\n" , "RFID_CARD_EVENT", lock_state_str[lock_operate.lock_state]);
		}



#endif		
		
		
				
		switch(lock_operate.lock_state)
		{		
			case 	LOCK_IDLE:
			if((WakeupFlag&0x03)!=0)//中断唤醒
			{
				lock_operate.lock_state = LOCK_READY;
			}
				break;
			case LOCK_ACTIVING:
				break;
			case LOCK_ERR:
				if(e.event!=EVENT_NONE)
				{
					HC595_Power_ON();
					SegDisplayCode = GetDisplayCodeFE();
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
					HC595_ShiftOut16(SER_LED_INTERFACE,LED_RED_ON_VALUE);
					
					Beep_Four_Time();
					HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);
					HC595_Power_OFF();
				}
				break;
			case LOCK_READY:

#if 1
				if(e.event==BUTTON_KEY_EVENT)
				{						
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Lock_EnterIdle();
							break;
						
						case KEY_DEL_SHORT:
							lock_operate.lock_action = DELETE_USER;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								if(Get_User_id_Number()>0)
								{
									lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
									SegDisplayCode = Lock_Enter_Wait_Delete_ID();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								else
								{
									Lock_NULL_Indication(); 
								}
							}
							else
							{
									fifo_clear(&touch_key_fifo);
									lock_operate.lock_state = WAIT_AUTHENTIC;
									SegDisplayCode = GetDisplayCodeAD();	
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							break;
							
						case KEY_ADD_SHORT:
							lock_operate.lock_action = ADD_USER;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								if(Get_User_id_Number()<95)
								{
									gOperateBit =0;
									lock_operate.id = Find_Next_User_Null_ID_Add(0);  
									lock_operate.lock_state = WAIT_SELECT_USER_ID;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								else
									Lock_FU_Indication();
							}	
							else
							{
								fifo_clear(&touch_key_fifo);
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.lock_state = WAIT_AUTHENTIC;
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}		
							break;
						case KEY_DEL_LONG:
							lock_operate.lock_action = DELETE_ADMIN;
							if(Get_Admin_id_Number()!=0)  //大于0， 肯定在SECURITY mode下
							{
								lock_operate.plock_infor->work_mode=SECURITY;
								fifo_clear(&touch_key_fifo);
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							else 
							{
								Lock_NULL_Indication();
							}
							break;
							
						case KEY_ADD_LONG:
							lock_operate.lock_action = ADD_ADMIN;
							if(Get_Admin_id_Number()>0)
							{
								fifo_clear(&touch_key_fifo);
								lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							else
							{
								if(Get_Admin_id_Number()>=4)
									Lock_FU_Indication();
								else
								{
									gOperateBit =0;
									fifo_clear(&touch_key_fifo);
									lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
									lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
							}
							break;		
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							lock_operate.lock_action = ADD_USER;	
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								if(Get_User_id_Number()<95)
								{
									fifo_clear(&touch_key_fifo);
									lock_operate.id = Find_Next_User_Null_ID_Add(0);
									lock_operate.lock_state = WAIT_PASSWORD_ONE;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
								}
								else
								{
									Lock_FU_Indication();
								}
							}	
							else
							{
								fifo_clear(&touch_key_fifo);
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.id = 0;
								lock_operate.lock_state = WAIT_AUTHENTIC;
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
//								printf("-s LOCK_READY -e KEY_OK -a WAIT_AUTHENTIC\r\n");
							}		

							break;
							
						case KEY_INIT_LONG:

								lock_operate.lock_action = DELETE_ALL;
								if(lock_operate.plock_infor->work_mode==NORMAL)
								{
									if(Get_id_Number()>0)
									{
										Action_Delete_All_ID();
									}	
									else
									{
										Lock_NULL_Indication();
									}
								}
								else
								{
									fifo_clear(&touch_key_fifo);
									lock_operate.lock_state = WAIT_AUTHENTIC;
									SegDisplayCode = GetDisplayCodeAD();	
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );									
								}
							break;
						
						default:
							break;
					}
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
						uint8_t len;
					
						id = 0;
						len = Get_fifo_size(&touch_key_fifo);
					  if((len==1)&&(e.data.KeyValude==('#'|LONG_KEY_MASK))) ///long press '#' to enter Normal open
						{
							if(lock_operate.pDooInfor->door_mode==0)
								Enter_Open_Normally_Mode();
							else
								Enter_Close_Normally_Mode();
							
							fifo_clear(&touch_key_fifo);
							
						}
						if(e.data.KeyValude=='*')
						{
							if(len>1)
							{
//								Touch_Once__Warm();
								fifo_clear(&touch_key_fifo);
							}
							else
								Lock_EnterIdle();
						}
						if((len>=TOUCH_KEY_PSWD_MAX_LEN)||(e.data.KeyValude=='#'))
						{
							if(len>=TOUCH_KEY_PSWD_LEN)
							{
								touch_key_buf[len] = '\0';
								id = Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf);
								if(id!=0)
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//需要确认之后的状态
									PASSWD_SUCESS_ON();
								//	if(lock_operate.pDooInfor->door_mode==0)//正常模式
									{
	//									if(lock_operate.pDooInfor->door_state==0) //关闭状态
											lock_operate.lock_state = LOCK_OPEN_CLOSE;	
									}
									fifo_clear(&touch_key_fifo);
								}
								else 
								{
									PW_Err_Count++;
									if(PW_Err_Count>=3)
									{
										Lock_Enter_Err();
									}
									else
									{
										PASSWD_COMPARE_ERR();
									}
									fifo_clear(&touch_key_fifo);
								}
							}
							else
							{
									PW_Err_Count++;
									printf("err %d\r\n", PW_Err_Count);
									if(PW_Err_Count>=3)
									{
										Lock_Enter_Err();
									}
									else
									{
										fifo_clear(&touch_key_fifo);
										PASSWD_COMPARE_ERR();
									}
							}
						}	
				}
				else if(e.event==RFID_CARD_EVENT)
				{
						id = Compare_To_Flash_id(RFID_PSWD, (char*)e.data.Buff);
						if(id!=0)
						{
							PW_Err_Count = 0;
							lock_operate.id = id;
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//需要确认之后的状态
							PASSWD_SUCESS_ON();
							//if(lock_operate.pDooInfor->door_mode==0)//正常模式
								lock_operate.lock_state = LOCK_OPEN_CLOSE;
						}
						else 
						{
							PW_Err_Count++;
							if(PW_Err_Count>=3)
							{
								Lock_Enter_Err();
							}
							else
								PASSWD_COMPARE_ERR();	
						}
					
				}
			#endif
				break;
			case WAIT_SELECT_DELETE_MODE:
			{
				
				int8_t (*pFun)(int8_t x);
				uint8_t id_l, id_h;
				
				if(lock_operate.lock_action == DELETE_ADMIN)
				{
					id_l = USER_ID_MAX;
					id_h = ADMIN_ID_MAX;
					pFun = Find_Next_Admin_ID_Add;
					
				}
				else
				{
					id_l = 0;
					id_h = USER_ID_MAX;
					pFun = Find_Next_User_ID_Add;
				}
				
				if(e.event==BUTTON_KEY_EVENT) 
				{
					gOperateBit =0;
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
							SegDisplayCode = Lock_EnterReady();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
							break;
						case KEY_DEL_SHORT:
						if(lock_operate.lock_action==DELETE_ADMIN)	
						{
								if(Delete_Mode_Temp==DELETE_ADMIN_BY_FP)
								{
										Delete_Mode_Temp = DELETE_ADMIN_ALL;
										SegDisplayCode = GetDisplayCodeAL();
								}
								else if(Delete_Mode_Temp == DELETE_ADMIN_ALL)
								{
									id  = Find_Next_Admin_ID_Dec(0);
									if(id!=-1)
									{
										lock_operate.id = id;
										Delete_Mode_Temp=DELETE_ADMIN_ID;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{ //如果没有ID将不会运行到这里，所以这是个错误的状态
										SegDisplayCode = GetDisplayCodeNull();   
										Hal_Beep_Blink (20, 600, 600);  //需要看效果 
										lock_operate.lock_state = LOCK_READY;
									}
								}
								else if(Delete_Mode_Temp==DELETE_ADMIN_ID)
								{
									id  = Find_Next_Admin_ID_Dec(lock_operate.id);
									if(id!=-1)
									{
										lock_operate.id = id;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{//如果没有ID将不会运行到这里，所以这是个错误的状态
										Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
										SegDisplayCode = GetDisplayCodeFP();
									}
								}
								else  //delete 状态之外，非法状态
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									Hal_Beep_Blink (20, 600, 600);  //需要看效果
									SegDisplayCode = GetDisplayCodeFP();
								}
								if(lock_operate.lock_state==LOCK_READY)
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
								else
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
						else if(lock_operate.lock_action==DELETE_USER)	
						{
							if(Delete_Mode_Temp==DELETE_USER_BY_FP)
							{
									Delete_Mode_Temp = DELETE_USER_ALL;
									SegDisplayCode = GetDisplayCodeAL();
							}
							else if(Delete_Mode_Temp == DELETE_USER_ALL)
							{
									
									id  = Find_Next_User_ID_Dec(0);
									if(id!=-1)
									{
										lock_operate.id = id;
										Delete_Mode_Temp=DELETE_USER_ID;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();   
										Hal_Beep_Blink (20, 600, 600);  //需要看效果
										lock_operate.lock_state = LOCK_READY;
									}
							}
							else if(Delete_Mode_Temp==DELETE_USER_ID)
							{
								id  = Find_Next_User_ID_Dec(lock_operate.id);
								if(id!=-1)
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(id);
								}
								else
								{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									SegDisplayCode = GetDisplayCodeFP();
								}
								
							}
							else  //delete 状态之外，非法状态
							{
								Delete_Mode_Temp = DELETE_USER_BY_FP;
								Hal_Beep_Blink (20, 600, 600);  //需要看效果
								SegDisplayCode = GetDisplayCodeFP();
							}
							if(lock_operate.lock_state==LOCK_READY)
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							else
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
							break;
					case KEY_ADD_SHORT:
						if(lock_operate.lock_action==DELETE_ADMIN)	
						{
								if(Delete_Mode_Temp==DELETE_ADMIN_BY_FP)
								{
									id  = Find_Next_Admin_ID_Add(0);
									if(id!=-1)
									{
										lock_operate.id = id;
										Delete_Mode_Temp=DELETE_ADMIN_ID;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();   
										Hal_Beep_Blink (20, 600, 600);  //需要看效果
										lock_operate.lock_state = LOCK_READY;
									}

								}
								else if(Delete_Mode_Temp == DELETE_ADMIN_ALL)
								{
										Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
										SegDisplayCode = GetDisplayCodeFP();
								}
								else if(Delete_Mode_Temp==DELETE_ADMIN_ID)
								{
									id  = Find_Next_Admin_ID_Add(lock_operate.id);
									if(id!=-1)
									{
										lock_operate.id = id;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{
										Delete_Mode_Temp = DELETE_ADMIN_ALL;
										SegDisplayCode = GetDisplayCodeAL();

									}
								}
								else  //delete 状态之外，非法状态
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									Hal_Beep_Blink (20, 600, 600);  //需要看效果
									SegDisplayCode = GetDisplayCodeFP();
								}
							if(lock_operate.lock_state==LOCK_READY)
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							else
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
						else if(lock_operate.lock_action==DELETE_USER)	
						{
							if(Delete_Mode_Temp==DELETE_USER_BY_FP)
							{							
									id  = Find_Next_User_ID_Dec(0);
									if(id!=-1)
									{
										lock_operate.id = id;
										Delete_Mode_Temp=DELETE_USER_ID;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();   
										Hal_Beep_Blink (20, 600, 600);  //需要看效果
										lock_operate.lock_state = LOCK_READY;
									}
							}
							else if(Delete_Mode_Temp == DELETE_USER_ALL)
							{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									SegDisplayCode = GetDisplayCodeFP();
							}
							else if(Delete_Mode_Temp==DELETE_USER_ID)
							{
								id  = Find_Next_User_ID_Dec(lock_operate.id);
								if(id!=-1)
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(id);
								}
								else
								{							
									Delete_Mode_Temp = DELETE_USER_ALL;
									SegDisplayCode = GetDisplayCodeAL();
								}
								
							}
							else  //delete 状态之外，非法状态
							{
								Delete_Mode_Temp = DELETE_USER_BY_FP;
								Hal_Beep_Blink (20, 600, 600);  //需要看效果
								SegDisplayCode = GetDisplayCodeFP();
							}
							if(lock_operate.lock_state==LOCK_READY)
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							else
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
						break;
					case KEY_OK_SHORT:
						if(Delete_Mode_Temp == DELETE_USER_BY_FP)
						{
							lock_operate.lock_state = DELETE_USER_BY_FP;
							SegDisplayCode = Lock_Enter_DELETE_USER_BY_FP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );  
						}
						else if(Delete_Mode_Temp == DELETE_USER_ALL)
						{
							Action_Delete_All_ID();
						}
						else if(Delete_Mode_Temp == DELETE_USER_ID)
						{
							if((lock_operate.id>id_l)&&(lock_operate.id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									PASSWD_Delete_ONE_ID(); 
									Delect_Index(lock_operate.id);
									if(Get_User_id_Number()==0)
											Lock_NULL_Indication();
									SegDisplayCode = GetDisplayCodeFP();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
								}
								else 
								{//错误状态
									gOperateBit =0;
									BIT_MORE_TWO_WARM();          /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
									lock_operate.id = pFun(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								
							}
							else
							{ //错误状态
								gOperateBit =0;
								BIT_MORE_TWO_WARM();
								lock_operate.id = pFun(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}

						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_BY_FP)
						{
							SegDisplayCode = Lock_Enter_DELETE_ADMIN_BY_FP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_ALL)
						{
							Action_Delete_All_ID();
						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_ID)
						{
							if((lock_operate.id>id_l)&&(lock_operate.id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									PASSWD_Delete_ONE_ID();  
									Delect_Index(lock_operate.id);
									if(Get_Admin_id_Number()==0)
									{
										lock_infor.work_mode = NORMAL;
										Index_Save();
										Lock_NULL_Indication();
									}
									SegDisplayCode = GetDisplayCodeFP();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
								}
								else 
								{//Err
									gOperateBit =0;
									Beep_Null_Warm();
									lock_operate.id = pFun(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								
							}
							else
							{//Err
									gOperateBit =0;
									Beep_Null_Warm();
									lock_operate.id = pFun(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{//Err
							Hal_Beep_Blink (20, 600, 600); 
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
						}
						
//						if((Delete_Mode_Temp == DELETE_ADMIN_BY_FP)||(Delete_Mode_Temp == DELETE_USER_BY_FP)||(Delete_Mode_Temp ==DELETE_USER_ID))
//							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );  ///注意
//						else
//							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );  ///注意
						break;
					case KEY_INIT_SHORT:
					default:
						break;
				}
				
			}
			else if(e.event==TOUCH_KEY_EVENT) //其他的状态下* # 可以做确定用，所以此处不当
			{
				if((Delete_Mode_Temp == DELETE_ADMIN_ID)||(Delete_Mode_Temp ==DELETE_USER_ID))
				
					if((e.data.KeyValude>=0x30)&&(e.data.KeyValude<=0x39))
					{
						if(gOperateBit==0)
						{
							gOperateBit ++; //1
							lock_operate.id = e.data.KeyValude -0x30;
						}
						else if(gOperateBit==1)
						{
							gOperateBit ++; //2
							lock_operate.id = (lock_operate.id*10) + (e.data.KeyValude-0x30);
						}
						else
						{
							gOperateBit =0;
							BIT_MORE_TWO_WARM();///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
					}
					else if(e.data.KeyValude=='#')
					{
						if((id>id_l)&&(id<=id_h))
						{
							if(pFun(lock_operate.id-1)==lock_operate.id)
							{
								fifo_clear(&touch_key_fifo);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							else 
							{
								gOperateBit =0;
								Beep_Null_Warm();
								lock_operate.id = pFun(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{
							gOperateBit =0;
							Beep_Null_Warm();
							lock_operate.id = pFun(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
					}
					else if((e.data.KeyValude=='*'))
					{
							SegDisplayCode = Lock_EnterReady();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
					}	
			}
			break;
		}
			case WAIT_SELECT_USER_ID:
				if(e.event==BUTTON_KEY_EVENT) 
				{
						switch (e.data.KeyValude)
						{
							case KEY_CANCEL_SHORT:
								SegDisplayCode = Lock_EnterReady();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
								break;
							case KEY_DEL_SHORT:
								id = Find_Next_User_Null_ID_Dec(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()>=95)
									{
										Lock_FU_Indication();
									}
									else
									{
										lock_operate.id = Find_Next_User_Null_ID_Dec(96);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
								}
								else
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//									printf("-s WAIT_SELECT_USER_ID -e KEY_DEL_SHORT -id %d\r\n",lock_operate.id);
								}
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_ADD_SHORT:
								if(lock_operate.id>=95)
									lock_operate.id=0;
								id = Find_Next_User_Null_ID_Add(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()>=95)
									{
										Lock_FU_Indication();
									//	lock_operate.lock_state = LOCK_IDLE;
									}
									else
									{
										lock_operate.id = Find_Next_User_Null_ID_Add(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										
									}
								}
								else
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								}
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_OK_SHORT:							
								if((lock_operate.id>0)&&(lock_operate.id<=USER_ID_MAX))
								{
									if(Find_Next_User_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
									{
										fifo_clear(&touch_key_fifo);
										lock_operate.lock_state = WAIT_PASSWORD_ONE;
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
									}
									else 
									{
										gOperateBit =0;
										PASSWD_ID_OVER_Indication();	
										lock_operate.id = Find_Next_User_Null_ID_Add(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
									}
								}
								else
								{
									gOperateBit =0;
									PASSWD_ID_OVER_Indication();	
									lock_operate.id = Find_Next_User_Null_ID_Add(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
							
//								printf("-s WAIT_SELECT_USER_ID -e KEY_OK -id %d \r\n",lock_operate.id);
								break;
							case KEY_INIT_SHORT:
				//			case KEY_INIT_LONG:
								break;
							default :
								break;
						}
						if((lock_operate.lock_state!=LOCK_IDLE)&&(lock_operate.lock_state!=LOCK_READY)&&(lock_operate.lock_state!=WAIT_PASSWORD_ONE))
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
					if((e.data.KeyValude>='0')&&(e.data.KeyValude<='9'))/*0~9*/
					{
						if(gOperateBit==0)
						{
							gOperateBit ++; //1
							lock_operate.id = e.data.KeyValude -0x30;
						}
						else if(gOperateBit==1)
						{
							gOperateBit ++; //2
							lock_operate.id = (lock_operate.id*10) + (e.data.KeyValude-0x30);
							
						}
						else
						{
							gOperateBit =0;
							BIT_MORE_TWO_WARM();
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
					}
					else if(e.data.KeyValude=='#')
					{
						if((id>0)&&(id<=USER_ID_MAX))
						{
							if(Find_Next_User_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
							{
								fifo_clear(&touch_key_fifo);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							else 
							{
								gOperateBit =0;
								PASSWD_ID_OVER_Indication();	
								lock_operate.id = Find_Next_User_Null_ID_Add(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{
							gOperateBit =0;
							PASSWD_ID_OVER_Indication();	
							lock_operate.id = Find_Next_User_Null_ID_Add(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
					}
					else if(e.data.KeyValude=='*')
					{
						SegDisplayCode = Lock_EnterReady();
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
					}		
				}
				
				break;
			case WATI_SELECT_ADMIN_ID:
				if(e.event==BUTTON_KEY_EVENT) 
				{
						switch (e.data.KeyValude)
						{
							case KEY_CANCEL_SHORT:
							case KEY_CANCEL_LONG:
								SegDisplayCode = Lock_EnterReady();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
								break;
							case KEY_DEL_SHORT:
							case KEY_DEL_LONG:
								if(lock_operate.id<=96)
									lock_operate.id=100;		
								id = Find_Next_Admin_Null_ID_Dec(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_Admin_id_Number()>=4)
									{
										Lock_FU_Indication();
									}
									else
									{
										lock_operate.id = Find_Next_Admin_Null_ID_Dec(100);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
								}
								else
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								}
								break;
							case KEY_ADD_SHORT:
							case KEY_ADD_LONG:
								if(lock_operate.id>=99)
									lock_operate.id=95;
								id = Find_Next_Admin_Null_ID_Add(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()>=4)
									{
										Lock_FU_Indication();
									}
									else
									{
										lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
								}
								else
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								}
								break;
							case KEY_OK_SHORT:
							case KEY_OK_LONG:		
								if((lock_operate.id>USER_ID_MAX)&&(lock_operate.id<=ADMIN_ID_MAX))
								{
									if(Find_Next_Admin_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
									{
										fifo_clear(&touch_key_fifo);
										lock_operate.lock_state = WAIT_PASSWORD_ONE;
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
									}
									else 
									{
										gOperateBit =0;
										PASSWD_ID_OVER_Indication();	
										lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
									}
								}
								else
								{
									gOperateBit =0;
									PASSWD_ID_OVER_Indication();	
									lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								break;
							case KEY_INIT_SHORT:
							case KEY_INIT_LONG:
								break;
							default :
								break;
						}
						if((lock_operate.lock_state!=LOCK_READY)&&(lock_operate.lock_state!=WAIT_PASSWORD_ONE))
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
					if((e.data.KeyValude>=0x30)&&(e.data.KeyValude<=0x39))
					{
						if(gOperateBit==0)
						{
							gOperateBit ++; //1
							lock_operate.id = e.data.KeyValude -0x30;
						}
						else if(gOperateBit==1)
						{
							gOperateBit ++; //2
							lock_operate.id = (lock_operate.id*10) + (e.data.KeyValude-0x30);
						}
						else
						{
							gOperateBit =0;
							BIT_MORE_TWO_WARM();
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
					}
					else if(e.data.KeyValude=='#')
					{
						if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
						{
							if(Find_Next_Admin_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
							{
								fifo_clear(&touch_key_fifo);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							}
							else 
							{
								gOperateBit =0;
								PASSWD_ID_OVER_Indication();	
								lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{
							gOperateBit =0;
							PASSWD_ID_OVER_Indication();	
							lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
				}
				else if(e.data.KeyValude=='*')
				{
					SegDisplayCode = Lock_EnterReady();
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
				}			
			}
				
				break;

			case WAIT_PASSWORD_ONE:
				if(e.event==BUTTON_KEY_EVENT) 
				{
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							if((lock_operate.id>=96) && (lock_operate.id<100))
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
							else
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
							gOperateBit =0;
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							break;
						default :
							break;
					}
				}	
				else if(e.event==TOUCH_KEY_EVENT)
				{
					uint8_t len;
					
					len = Get_fifo_size(&touch_key_fifo);
					if((len==TOUCH_KEY_PSWD_LEN)&&(!((e.data.KeyValude=='#')||(e.data.KeyValude=='*'))))
					{				
							
							touch_key_buf[len] = '\0';
							if(Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf)==0)
							{
								PASSWD_ONE_OK();
								gEventOne.event = TOUCH_KEY_EVENT;
								strcpy(gEventOne.data.Buff, touch_key_buf);
								lock_operate.lock_state = WATI_PASSWORD_TWO;
							}
							else
								Beep_Register_Fail_Warm();//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							fifo_clear(&touch_key_fifo);
					}
					else
					{
						if(e.data.KeyValude=='#')
						{
							fifo_clear(&touch_key_fifo);
							Beep_Register_Fail_Warm();
						}
						else if(e.data.KeyValude=='*')
						{
							if(len>1)
							{
								fifo_clear(&touch_key_fifo);
//								Touch_Once__Warm(); ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							}
							else
							{
								if((lock_operate.id>=96) && (lock_operate.id<100))
									lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
								else
									lock_operate.lock_state = WAIT_SELECT_USER_ID;
								gOperateBit =0;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							}	
						}
						else
						{
//							Beep_Register_Fail_Warm();
//							fifo_clear(&touch_key_fifo);
						}
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					if(Compare_To_Flash_id(RFID_PSWD, (char*)e.data.Buff)==0)
					{
						PASSWD_ONE_OK();
						gEventOne.event = RFID_CARD_EVENT;
						strcpy(gEventOne.data.Buff, e.data.Buff);
						lock_operate.lock_state = WATI_PASSWORD_TWO;
					}
					else
						Beep_Register_Fail_Warm();    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					fifo_clear(&touch_key_fifo);
				}
				break;
			case WATI_PASSWORD_TWO:
				if(e.event==BUTTON_KEY_EVENT) 
				{
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							if((lock_operate.id>=96) && (lock_operate.id<100))
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
							else
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
							gOperateBit =0;
						//lock_operate.lock_state = WAIT_SELECT_USER_ID;
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							break;
						default :
							break;
					}
				}
				else if(e.event==TOUCH_KEY_EVENT)//should add button operate
				{		
					uint8_t len;
					id_infor_t id_infor;
					
					len = Get_fifo_size(&touch_key_fifo);
					if((len==TOUCH_KEY_PSWD_LEN)&&(!((e.data.KeyValude=='#')||(e.data.KeyValude=='*'))))
					{
						touch_key_buf[len] = '\0';
						if((gEventOne.event==TOUCH_KEY_EVENT)&&(strcmp(touch_key_buf, gEventOne.data.Buff)==0))
						{
							id_infor.id = lock_operate.id;
							id_infor.type = TOUCH_PSWD;
							id_infor.len = TOUCH_KEY_PSWD_LEN;
							strcpy(id_infor.password, touch_key_buf);	
							id_infor_Save(lock_operate.id, id_infor);
							Add_Index(lock_operate.id);
							PASSWD_TWO_OK();
							if((lock_operate.id>=96) && (lock_operate.id<100))
							{
								if(Get_Admin_id_Number()>=4)
									Lock_FU_Indication();
								else
								{
									lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
									id = Find_Next_Admin_Null_ID_Add(lock_operate.id);
								}
							}
							else
							{
								if(Get_User_id_Number()>=95)
									Lock_FU_Indication();
								else
								{
									lock_operate.lock_state = WAIT_SELECT_USER_ID;
									id = Find_Next_User_Null_ID_Add(lock_operate.id);
								}
							}
							gOperateBit =0;
							if(id==-1)
							{
								id = Find_Next_User_Null_ID_Add(0);//找到一个空ID
							}
							if(id!=-1)  
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								
							}
							else//////////////如果ID依旧为空，，说明锁的状态跑错了
							{
								lock_operate.id = 100; 
								SegDisplayCode = GetDisplayCodeFU();
								Hal_Beep_Blink (20, 600, 600);  //需要看效果 
							}
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );	
						}
						else
						{
							lock_operate.lock_state = WAIT_PASSWORD_ONE;
							Beep_Register_Fail_Warm(); ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							//toddo
						}
						memset(&gEventOne, 0, sizeof(EventDataTypeDef));
						fifo_clear(&touch_key_fifo);
					}
					else if(e.data.KeyValude=='#')
					{
							lock_operate.lock_state = WAIT_PASSWORD_ONE;
							fifo_clear(&touch_key_fifo);
							Beep_Register_Fail_Warm(); /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					}
					else if(e.data.KeyValude=='*')
					{
						if(len>1)
						{
							fifo_clear(&touch_key_fifo);
//							Touch_Once__Warm();
						}
						else
						{
							if((lock_operate.id>=96) && (lock_operate.id<100))
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
							else
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
							gOperateBit =0;
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}	
					}
			  }
				else if(e.event==RFID_CARD_EVENT)
				{
						id_infor_t id_infor;
					
						if((gEventOne.event==RFID_CARD_EVENT)&&(strcmp(e.data.Buff, gEventOne.data.Buff)==0))
						{
							id_infor.id = lock_operate.id;
							id_infor.type = RFID_PSWD;
							id_infor.len = RFID_CARD_NUM_LEN;
							strcpy(id_infor.password, e.data.Buff);	
							id_infor_Save(lock_operate.id, id_infor);
							Add_Index(lock_operate.id);
							PASSWD_TWO_OK();
							if((lock_operate.id>=96) && (lock_operate.id<100))
							{
								if(Get_Admin_id_Number()>=4)
									Lock_FU_Indication();
								else
								{
									lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
									id = Find_Next_Admin_Null_ID_Add(lock_operate.id);
								}
							}
							else
							{
								if(Get_User_id_Number()>=95)
									Lock_FU_Indication();
								else
								{
									lock_operate.lock_state = WAIT_SELECT_USER_ID;
									id = Find_Next_User_Null_ID_Add(lock_operate.id);
								}
							}
							gOperateBit =0;
							
							if(id==-1)
							{
								id = Find_Next_User_Null_ID_Add(0);//找到一个空ID
							}
							if(id!=-1)
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								
							}
							else
							{
								lock_operate.id = 100; 
								SegDisplayCode = GetDisplayCodeFU();
								Hal_Beep_Blink (20, 600, 600);  //需要看效果 
							}
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );	
						}
						else
						{
							lock_operate.lock_state = WAIT_PASSWORD_ONE;
							Beep_Register_Fail_Warm(); ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							//toddo
						}
						memset(&gEventOne, 0, sizeof(EventDataTypeDef));
						fifo_clear(&touch_key_fifo);
				}
				break;
			case WAIT_AUTHENTIC:  // should add button operate
				if(e.event==BUTTON_KEY_EVENT) 
				{
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
							SegDisplayCode = Lock_EnterReady();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
							break;
						default :
							break;
					}
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{	
					uint8_t len;
					
					len = Get_fifo_size(&touch_key_fifo);
					if(e.data.KeyValude=='*')
					{
						if(len>1)
						{
							fifo_clear(&touch_key_fifo);
//							Touch_Once__Warm();
						}
						else
						{
							SegDisplayCode = Lock_EnterReady();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))||(e.data.KeyValude=='#'))
					{
							touch_key_buf[len] = '\0';
							if(0 !=Compare_To_Flash_Admin_id(TOUCH_PSWD, (char*)touch_key_buf))
							{
								PASSWD_COMPARE_OK();	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								if(lock_operate.lock_action == DELETE_USER)
								{
									if(Get_User_id_Number()!=0)
									{
										lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
										SegDisplayCode = Lock_Enter_Wait_Delete_ID();
									}
									else
									{
										Lock_NULL_Indication();
									}
								}
								else if(lock_operate.lock_action == DELETE_ADMIN)
								{
									lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
									SegDisplayCode = Lock_Enter_Wait_Delete_ID();
								}
								else if(lock_operate.lock_action == ADD_USER)
								{
									if(Get_User_id_Number()<95)
									{
										gOperateBit =0;
										lock_operate.id = Find_Next_User_Null_ID_Add(0);  
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										lock_operate.lock_state = WAIT_SELECT_USER_ID;
									}
									else
									{
										Lock_FU_Indication();
									}
								}
								else if(lock_operate.lock_action == ADD_ADMIN)
								{
									if(Get_Admin_id_Number()<4)
									{
										gOperateBit =0;
										lock_operate.id = Find_Next_Admin_Null_ID_Add(0);  
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
									}
									else
									{
										Lock_FU_Indication();
									}
								}
								else if(lock_operate.lock_action == DELETE_ALL)
								{								
									Action_Delete_All_ID();	
								}
								fifo_clear(&touch_key_fifo);
								if(lock_operate.lock_state!=LOCK_READY)
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//	
								else
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//	
							}					
							else
							{
								PASSWD_COMPARE_ERR();
								fifo_clear(&touch_key_fifo);
							}
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					if(0 !=Compare_To_Flash_Admin_id(RFID_PSWD, (char*)e.data.Buff))
					{
						PASSWD_COMPARE_OK();	
						if(lock_operate.lock_action == DELETE_USER)
						{
							if(Get_User_id_Number()!=0)
							{
								lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
								SegDisplayCode = Lock_Enter_Wait_Delete_ID();
							}
							else
							{
								Lock_NULL_Indication();
							}
						}
						else if(lock_operate.lock_action == DELETE_ADMIN) //里面肯定有admin id
						{
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = Lock_Enter_Wait_Delete_ID();
						}
						else if(lock_operate.lock_action == ADD_USER)
						{
							if(Get_User_id_Number()<95)
							{
								gOperateBit =0;
								lock_operate.id = Find_Next_User_Null_ID_Add(0);  
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
							}
							else
							{
								Lock_FU_Indication();
							}
						}
						else if(lock_operate.lock_action == ADD_ADMIN)
						{
							if(Get_Admin_id_Number()<4)
							{
								gOperateBit =0;
								lock_operate.id = Find_Next_Admin_Null_ID_Add(0);  
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
							}
							else
							{
								Lock_FU_Indication();
							}
						}
						else if(lock_operate.lock_action == DELETE_ALL)
						{
							if(Get_id_Number()>0)
							{
								Action_Delete_All_ID();
							}
							else
							{
								Lock_NULL_Indication();
							}
						}
						fifo_clear(&touch_key_fifo);
						if(lock_operate.lock_state!=LOCK_READY)
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//	
						else
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//	
					}	
					else
						PASSWD_COMPARE_ERR(); ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				}
				break;
			case DELETE_USER_BY_FP:
				if(e.event==BUTTON_KEY_EVENT)
				{

					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							break;
						case KEY_DEL_SHORT:
						case KEY_ADD_SHORT:
						case KEY_OK_SHORT:
						case KEY_INIT_SHORT:
							break;
						default:
								break;
					}
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
					uint8_t len;
					int8_t id;
					
					len = Get_fifo_size(&touch_key_fifo);
					if(e.data.KeyValude=='*')
					{
						if(len>1)
						{
							fifo_clear(&touch_key_fifo);
//							Touch_Once__Warm();
						}
						else
						{
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))||(e.data.KeyValude=='#'))
					{	
						touch_key_buf[len] = '\0';
						id = Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf);
						if(id !=0)//
						{
							if((id>0)&&(id<=USER_ID_MAX))
							{
								Delect_Index((uint8_t) id);
								PASSWD_Delete_ONE_ID();
								if(Get_User_id_Number()==0)
								{
									lock_infor.work_mode = NORMAL;
									Index_Save();
									Lock_NULL_Indication();
								}
							}
							else
							{
								Error_ID_Warm();
							}
							fifo_clear(&touch_key_fifo);
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}		
						else
						{
							Error_ID_Warm();
							fifo_clear(&touch_key_fifo);
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}
					}
					
				}
				else if(e.event==RFID_CARD_EVENT)
				{
						id = Compare_To_Flash_id(RFID_PSWD, (char*)e.data.Buff);
						if(id!=0)
						{
							if((id>0)&&(id<=USER_ID_MAX))
							{
								Delect_Index((uint8_t) id);
								PASSWD_Delete_ONE_ID();
								if(Get_User_id_Number()==0)
								{
									lock_infor.work_mode = NORMAL;
									//Index_Save();
									Lock_NULL_Indication();
								}
							}
							else
							{
								Error_ID_Warm();
							}
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}
						else 
							PASSWD_COMPARE_ERR(); 
						fifo_clear(&touch_key_fifo);
				}
					
				break;
			case DELETE_ADMIN_BY_FP:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							break;
						case KEY_DEL_SHORT:
						case KEY_ADD_SHORT:
						case KEY_OK_SHORT:
						case KEY_INIT_SHORT:
							break;
						default:
								break;
					}
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
					uint8_t len;
					int8_t id;
					
					len = Get_fifo_size(&touch_key_fifo);
				  if(e.data.KeyValude=='*')
					{
						if(len>1)
						{
							fifo_clear(&touch_key_fifo);
//							Touch_Once__Warm();
						}
						else
						{
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))||(e.data.KeyValude=='#'))
					{
						touch_key_buf[len] = '\0';
						id = Compare_To_Flash_Admin_id(TOUCH_PSWD, (char*)touch_key_buf);
						if(id !=0)
						{
							if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
							{
								PASSWD_Delete_ONE_ID();//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								Delect_Index((uint8_t) id);
								if(Get_Admin_id_Number()==0)
								{
									lock_infor.work_mode = NORMAL;
									Index_Save();
									Lock_NULL_Indication();
								}
							}
							else
								Error_ID_Warm();
							
							fifo_clear(&touch_key_fifo);
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}	
						else
						{
							Error_ID_Warm();  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}						
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					  id = Compare_To_Flash_Admin_id(RFID_PSWD, (char*)e.data.Buff);
						if(id !=0)
						{
							if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
							{
								PASSWD_Delete_ONE_ID();
								Delect_Index((uint8_t) id);
								if(Get_Admin_id_Number()==0)
								{
									lock_infor.work_mode = NORMAL;
									Index_Save();
									Lock_NULL_Indication();
								}
							}
							else
								Error_ID_Warm();
							
							fifo_clear(&touch_key_fifo);
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}		
				}
					
				break;
			case ADD_ID_OK:
			case DELETE_ID_OK:
				break;
			case LOCK_OPEN:
				if(motor_state==MOTOR_NONE)
				{
					motor_state = MOTOR_FORWARDK;
					MotorEndTime = GetSystemTime() + 500/2;
					Motor_Drive_Forward();
				}
				else
				{
					if(GetSystemTime() > MotorEndTime)
					{
						motor_state = MOTOR_NONE;
						lock_operate.pDooInfor->door_state = 1;
						Motor_Drive_Stop();
						Save_DoorInfor(lock_operate.pDooInfor);
//						SegDisplayCode = Lock_EnterReady();
						SegDisplayCode = GetDisplayCodeActive();
						fifo_clear(&touch_key_fifo);
						lock_operate.lock_state = LOCK_READY;
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//显示--或者u n
					}
				}
	
				break;
			case LOCK_CLOSE:
				if(motor_state==MOTOR_NONE)
				{
					motor_state = MOTOR_REVERSE;
					MotorEndTime = GetSystemTime() + 500/2;
					printf("close time= %d\r\n",GetSystemTime());
					Motor_Drive_Reverse();
					
				}
				else
				{
					if(GetSystemTime() > MotorEndTime)
					{
						printf("close time= %d\r\n",GetSystemTime());
						motor_state = MOTOR_NONE;
						lock_operate.pDooInfor->door_state = 0;
						Motor_Drive_Stop();
						Save_DoorInfor(lock_operate.pDooInfor);
//						SegDisplayCode = Lock_EnterReady();
						SegDisplayCode = GetDisplayCodeActive();
						fifo_clear(&touch_key_fifo);
						lock_operate.lock_state = LOCK_READY;
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
					}
				}
				break;
			case LOCK_OPEN_CLOSE:
//				Motor_Drive_Forward();
//				delay_ms(500);

//				Motor_Drive_Stop();
//				delay_s(6);
//			
//				Motor_Drive_Reverse();
//				delay_ms(500);
//				
//			
//				motor_state = MOTOR_NONE;
//				lock_operate.pDooInfor->door_state = 1;
//				Motor_Drive_Stop();
//				Save_DoorInfor(lock_operate.pDooInfor);
//				Lock_EnterIdle();
			
				if(motor_state==MOTOR_NONE)
				{
					motor_state = MOTOR_FORWARDK;
					MotorEndTime = GetSystemTime() + 500/2;
					Motor_Drive_Forward();
				}
				else if(motor_state==MOTOR_FORWARDK)
				{
					if(GetSystemTime() >= MotorEndTime)
					{
						motor_state = MOTOR_STOP;
						lock_operate.pDooInfor->door_state = 0;
						MotorEndTime = GetSystemTime() + 3000/2;
						Motor_Drive_Stop();
						Save_DoorInfor(lock_operate.pDooInfor);
					}
				}
				else if(motor_state==MOTOR_STOP)
				{
					if(GetSystemTime() >= MotorEndTime)
					{
						motor_state = MOTOR_REVERSE;
						MotorEndTime = GetSystemTime() + 500/2;
						Motor_Drive_Reverse();
					}
				}
				else 
				{
					if(GetSystemTime() > MotorEndTime)
					{
						motor_state = MOTOR_NONE;
						lock_operate.pDooInfor->door_state = 1;
						Motor_Drive_Stop();
						Save_DoorInfor(lock_operate.pDooInfor);
						Lock_EnterIdle();
						//Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
					}
				}
				break;
			default :
				break;
	}
}


uint16_t GetDisplayCodeNull(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[19];   /* u */
	code = (code<<8) | LEDDisplayCode[20]; /*  displya u n*/				
	return code;
}

static uint16_t GetDisplayCodeAD(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[10];
	code = (code<<8) | LEDDisplayCode[13];/*  AD */
	return code;
}

static uint16_t GetDisplayCodeAL(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[10];
	code = (code<<8) | LEDDisplayCode[17];/*  AL */
	return code;
}
static uint16_t GetDisplayCodeFP(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[18];
	code = (code<<8) | LEDDisplayCode[15];/*  FP */
	return code;	
}

 uint16_t GetDisplayCodeActive(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[16];
	code = (code<<8) | LEDDisplayCode[16];/*  -- */
	return code;	
}

static uint16_t GetDisplayCodeNum(uint8_t num) /* Number */
{
	uint16_t code;
	
	if(num>99)
		return 0;
	
	code =   LEDDisplayCode[num/10];
	code = (code<<8) | LEDDisplayCode[num%10];	
	return code;	
}
static uint16_t GetDisplayCodeFU(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[19];
	code = (code<<8) | LEDDisplayCode[15];/*  FU */
	return code;	
}

static uint16_t GetDisplayCodeFE(void)  /*错误密码超出系统自锁*/
{
		uint16_t code;
	
	code = LEDDisplayCode[14];
	code = (code<<8) | LEDDisplayCode[15];/*  FU */
	return code;	
}



