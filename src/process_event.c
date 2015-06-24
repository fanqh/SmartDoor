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

#define SLEEP_TIMEOUT 50000/2  			  /* 定时器计时周期为 2ms */
#define Beep_Null_Warm()							{Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}  //id空报警
#define Beep_Touch_Tone()							{Hal_Beep_Blink (2, 100, 50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}  //touch 长度到提示
#define Beep_Fail_Warm()      				Hal_Beep_Blink (1, 50, 50) 
#define Beep_Delete_All_Warm()				Hal_Beep_Blink (4, 100, 100)
#define Beep_Delete_ID_Tone()					Hal_Beep_Blink (2, 100, 100)
#define Beep_Register_Fail_Warm() 		{Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}
#define Beep_Register_Sucess_Tone()   {Hal_Beep_Blink (3, 100,50);Hal_LED_Blink (LED_BLUE_ALL_ON_VALUE, 3, 200, 200);}
#define Beep_Compare_Fail_Warm()			Hal_Beep_Blink (2, 50,50)

#define Flash_ID_Full_Warm()					{Hal_Beep_Blink (2, 100, 100);Hal_LED_Blink (LED_RED_ON_VALUE, 3, 200, 200);}


#define Flash_Comare_Sucess_Warm()			 {Hal_Beep_Blink (3, 100,50);;Hal_LED_Blink (LED_BLUE_ALL_ON_VALUE, 3, 200, 200);}

#define Error_ID_Warm()								{Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 5, 200, 200);}
#define Comare_Fail_Warm()            {Hal_Beep_Blink (2, 50,50);Hal_LED_Blink (LED_RED_ON_VALUE, 5, 200, 200);}


#define LED_Blink_Compare_Fail_Warm()   Hal_LED_Blink (LED_RED_ON_VALUE, 5, 200, 200)  

#define DEBUG_  1

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_INIT,&lock_infor,0,0,0};
struct node_struct_t process_event_scan_node;
static void process_event(void);
static uint32_t SleepTime_End = 0; 
char gpswdOne[TOUCH_KEY_PSWD_LEN+1];
static Hal_EventTypedef gEventOne;
static LOCK_STATE Delete_Mode_Temp = DELETE_USER_BY_FP;
static uint8_t gOperateBit = 0;   //0  digits  1 decimal  2 warm  

static uint16_t GetDisplayCodeNull(void);
static uint16_t GetDisplayCodeAD(void);
static uint16_t GetDisplayCodeFP(void);
static uint16_t GetDisplayCodeActive(void);
static uint16_t GetDisplayCodeNum(uint8_t num);
static uint16_t GetDisplayCodeFU(void);
static uint16_t GetDisplayCodeAL(void);



static const char* lock_state_str[]=
{"INIT",
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
		lock_operate.lock_state = LOCK_INIT;
		lock_operate.user_num = 0;
		lock_operate.admin_num = 0;
		lklt_insert(&process_event_scan_node, process_event, NULL, 10/5);
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
//		printf("Init lock_state: LOCK_INIT\r\n");
}

uint16_t Lock_EnterIdle(void)
{
		//uint16_t SegDisplayCode;
	
		lock_operate.lock_state = LOCK_IDLE;
		fifo_clear(&touch_key_fifo);
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);//turn off SEG8_LED
		Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);  //turn off all led
//		printf("enter LOCK_IDLE\r\n");
		
	 return 0xffff;
}
uint16_t Lock_EnterReady(void)
{
	uint16_t SegDisplayCode;
	
	if(Get_id_Number()!=0)
	{
		SegDisplayCode = GetDisplayCodeActive();
	}
	else
	{
		SegDisplayCode = GetDisplayCodeNull();  
		Beep_Null_Warm();
	}			
	fifo_clear(&touch_key_fifo);
	lock_operate.lock_state = LOCK_READY;
	//Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
//	printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
	
	return SegDisplayCode;
}
uint16_t Lock_Enter_DELETE_USER_BY_FP(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_USER_BY_FP;
	code = GetDisplayCodeFP();
	return code;
}
uint16_t Lock_Enter_DELETE_USER_ID(void)
{
	uint16_t code;
	int8_t id;
	
	lock_operate.lock_state = DELETE_USER_ID;
	id  = Find_Next_User_ID_Dec(0);	
	if(id!=-1)
	{
		lock_operate.id = id;
		code = GetDisplayCodeNum(id);
	}
	else
		code = Lock_EnterIdle();//???
	return code;
}

uint16_t Lock_Enter_DELETE_USER_ALL(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_USER_ALL;
	code = GetDisplayCodeAL();
	return code;
}






uint16_t Lock_Enter_DELETE_ADMIN_BY_FP(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_ADMIN_BY_FP;
	code = GetDisplayCodeFP();
	return code;
}
uint16_t Lock_Enter_DELETE_ADMIN_ID(void)
{
	uint16_t code;
	int8_t id;
	
	lock_operate.lock_state = DELETE_ADMIN_ID;
	id  = Find_Next_Admin_ID_Add(0);	
	if(id!=-1)
	{
		lock_operate.id = id;
		code = GetDisplayCodeNum(id);
	}
	else
		code = Lock_Enter_DELETE_ADMIN_BY_FP();
	return code;
}

uint16_t Lock_Enter_DELETE_Admin_ALL(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_ADMIN_ALL;
	code = GetDisplayCodeAL();
	return code;
}


uint16_t Lock_Enter_Wait_Delete_ID(void)
{    
		uint16_t code;
	
//		lock_operate.id = 0;
		if(lock_operate.lock_action==DELETE_ADMIN)
			Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
		else
			Delete_Mode_Temp = DELETE_USER_BY_FP;
		code = GetDisplayCodeFP();
}

static void process_event(void)
{
	int8_t id;
	uint32_t time;
	uint16_t SegDisplayCode;
	Hal_EventTypedef e; 
	
		time= GetSystemTime();
	
		if((lock_operate.lock_state!=LOCK_IDLE)&&(time >= SleepTime_End))
				Lock_EnterIdle();																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																							
		e = USBH_GetEvent();
	  if((e.event==EVENT_NONE)&&(lock_operate.lock_state!=LOCK_INIT))
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
			case LOCK_INIT:
			{
					if(Get_id_Number()!=0)
					{
						 SegDisplayCode = GetDisplayCodeActive();
						
					}
					else
					{
						 SegDisplayCode = GetDisplayCodeNull();   
						 Beep_Null_Warm();
					
					}
					lock_operate.lock_state = LOCK_READY;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
//					printf("-s LOCK_INIT -e EVENT_NONE -a Lock_READY\r\n");
			}
			break;
			
			case 	LOCK_IDLE:
				if((e.event==BUTTON_KEY_EVENT) || (e.event==TOUCH_KEY_EVENT))
				{
					SegDisplayCode = Lock_EnterReady();
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
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
							SegDisplayCode = Lock_EnterIdle();
							//lock_operate.lock_state = LOCK_IDLE;
//							printf("-s LOCK_READY -e KEY_CANCEL_SHORT -a LOCK_IDLE\r\n");
							break;
						
						case KEY_DEL_SHORT:
							if(Get_User_id_Number()>0)
							{
								lock_operate.lock_action = DELETE_USER;
								if(lock_operate.plock_infor->work_mode==NORMAL)
								{
									lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
									SegDisplayCode = Lock_Enter_Wait_Delete_ID();
//									printf("-s LOCK_READY -e KEY_DEL_SHORT -a DELETE_USER_BY_FP\r\n");
								}
								else
								{
									lock_operate.lock_state = WAIT_AUTHENTIC;
									SegDisplayCode = GetDisplayCodeAD();
//									printf("-s LOCK_READY -e KEY_DEL_SHORT -a WAIT_AUTHENTIC\r\n");
								}
								
							}
							else
							{
								SegDisplayCode = GetDisplayCodeNull();   /* un */
								Beep_Null_Warm();
								lock_operate.lock_state = LOCK_READY;  
//								printf("-s LOCK_READY -e KEY_DEL_SHORT -a LOCK_READY\r\n");
							}
							break;
							
						case KEY_ADD_SHORT:
							lock_operate.lock_action = ADD_USER;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								gOperateBit =0;
								lock_operate.id = Find_Next_User_Null_ID_Add(0);  
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//								printf("-s LOCK_READY -e KEY_ADD_SHORT -a WAIT_SELECT_USER_ID -id :%d\r\n", lock_operate.id);
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.lock_state = WAIT_AUTHENTIC;
//								printf("-s LOCK_READY -e KEY_ADD_SHORT -a WAIT_AUTHENTIC\r\n");
							}						
							break;
						case KEY_DEL_LONG:
							lock_operate.lock_action = DELETE_ADMIN;
							if(Get_Admin_id_Number()!=0)
							{
								lock_operate.plock_infor->work_mode=SECURITY;
								
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
							}
							else 
							{
								lock_operate.lock_state = LOCK_READY;
								SegDisplayCode = GetDisplayCodeNull();   /* un */
								Beep_Null_Warm();
							}
							break;
							
						case KEY_ADD_LONG:
							lock_operate.lock_action = ADD_ADMIN;
							if(lock_operate.plock_infor->work_mode==SECURITY)
							{
								lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
							}
							else
							{
								gOperateBit =0;
								lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							}
							break;		
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							lock_operate.lock_action = ADD_USER;	
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_User_Null_ID_Add(0);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//								printf("-s LOCK_READY -e KEY_OK -a WAIT_PASSWORD_ONE\r\n");
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.id = 0;
								lock_operate.lock_state = WAIT_AUTHENTIC;
//								printf("-s LOCK_READY -e KEY_OK -a WAIT_AUTHENTIC\r\n");
							}		

							break;
							
						case KEY_INIT_LONG:
							break;
						
						default:
							break;
					}
					if(lock_operate.lock_state!=LOCK_IDLE)
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_BLINK, SegDisplayCode );
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
						uint8_t len;
					
						id = 0;
						len = Get_fifo_size(&touch_key_fifo);
						if((len>=TOUCH_KEY_PSWD_MAX_LEN)||(e.data.KeyValude=='*')||(e.data.KeyValude=='#'))
						{
							if(len>=TOUCH_KEY_PSWD_LEN)
							{
								touch_key_buf[len] = '\0';
								id = Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf);
								if(id!=0)
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//需要确认之后的状态
									Flash_Comare_Sucess_Warm();
									//需要加开锁
								}
								else 
								{
									fifo_clear(&touch_key_fifo);
									Comare_Fail_Warm(); 
								}
							}
							else
							{
									fifo_clear(&touch_key_fifo);
									Comare_Fail_Warm() ;
							}
						}
							
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					
				}
			#endif
				break;
			case WAIT_SELECT_DELETE_MODE:
			{
				
				int8_t (*pFun)(int8_t x);
				uint8_t id_l, id_h;
				
				if(Delete_Mode_Temp == DELETE_ADMIN_ID)
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
									{
										SegDisplayCode = GetDisplayCodeNull();   
										Beep_Null_Warm();
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
									{
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
										Beep_Null_Warm();
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
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
							break;
					case KEY_ADD_SHORT:
						if(lock_operate.lock_state==DELETE_ADMIN)	
						{
								if(Delete_Mode_Temp==DELETE_ADMIN_BY_FP)
								{
									id  = Find_Next_Admin_ID_Dec(0);
									if(id!=-1)
									{
										lock_operate.id = id;
										Delete_Mode_Temp=DELETE_ADMIN_ID;
										SegDisplayCode = GetDisplayCodeNum(id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();   
										Beep_Null_Warm();
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
									id  = Find_Next_Admin_ID_Dec(lock_operate.id);
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
										Beep_Null_Warm();
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
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
						break;
					case KEY_OK_SHORT:
						if(Delete_Mode_Temp == DELETE_USER_BY_FP)
						{
							lock_operate.lock_state = DELETE_USER_BY_FP;
							SegDisplayCode = Lock_Enter_DELETE_USER_BY_FP();
						}
						else if(Delete_Mode_Temp == DELETE_USER_ALL)
						{
							Beep_Delete_All_Warm(); 
							Erase_All_User_id();
							SegDisplayCode = Lock_EnterReady();  //状态到ready
						}
						else if(Delete_Mode_Temp == DELETE_USER_ID)
						{
							if((id>id_l)&&(id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									Beep_Delete_ID_Tone(); 
									Delect_Index(lock_operate.id);
									SegDisplayCode = GetDisplayCodeFP();
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
						else if(Delete_Mode_Temp == DELETE_ADMIN_BY_FP)
						{
							SegDisplayCode = Lock_Enter_DELETE_ADMIN_BY_FP();
						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_ALL)
						{
							Beep_Delete_All_Warm();
							Erase_All_Admin_id();
							SegDisplayCode = Lock_EnterReady();  //状态到ready
						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_ID)
						{
							if((id>id_l)&&(id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									Beep_Delete_ID_Tone(); 
									Delect_Index(lock_operate.id);
									SegDisplayCode = GetDisplayCodeFP();
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
						else
						{
							Hal_Beep_Blink (20, 600, 600); 
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							SegDisplayCode = GetDisplayCodeFP();
						}
						
						if((Delete_Mode_Temp == DELETE_ADMIN_BY_FP)||(Delete_Mode_Temp == DELETE_USER_BY_FP)||(Delete_Mode_Temp ==DELETE_USER_ID))
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );  ///注意
						else
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );  ///注意
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
							Beep_Null_Warm();
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
					}
					else if((e.data.KeyValude==0x23) || (e.data.KeyValude==0x2a))
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
					else
					{}	
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
//								printf("-s WAIT_SELECT_USER_ID -e KEY_CANCEL_LONG -a LOCK_READY\r\n");
								break;
							case KEY_DEL_SHORT:
								id = Find_Next_User_Null_ID_Dec(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()==95)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Flash_ID_Full_Warm();
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
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
									if(Get_User_id_Number()==95)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Flash_ID_Full_Warm();
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
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
//									printf("-s WAIT_SELECT_USER_ID -e KEY_ADD -id %d\r\n",lock_operate.id);
								}
//								printf("-s WAIT_SELECT_USER_ID -e KEY_ADD -id %d\r\n",lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_OK_SHORT:
//								fifo_clear(&touch_key_fifo);
//								lock_operate.lock_state = WAIT_PASSWORD_ONE;
//								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							
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
										Beep_Null_Warm();
										lock_operate.id = Find_Next_User_Null_ID_Add(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
									}
								}
								else
								{
									gOperateBit =0;
									Beep_Null_Warm();
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
//							if(Find_Next_User_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
//							{
//								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
//							}
//							else
//							{
//								gOperateBit =0;
//								Beep_Null_Warm();
//								lock_operate.id = Find_Next_User_Null_ID_Add(0);
//							}
						}
						else
						{
							gOperateBit =0;
							Beep_Null_Warm();
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
					}
					else if((e.data.KeyValude==0x23) || (e.data.KeyValude==0x2a))
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
								Beep_Null_Warm();
								lock_operate.id = Find_Next_User_Null_ID_Add(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{
							gOperateBit =0;
							Beep_Null_Warm();
							lock_operate.id = Find_Next_User_Null_ID_Add(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
					}
					else
					{}		
				}
				
				break;
			case WATI_SELECT_ADMIN_ID:
				if(e.event==BUTTON_KEY_EVENT) 
				{
						switch (e.data.KeyValude)
						{
							case KEY_CANCEL_SHORT:
							case KEY_CANCEL_LONG:
//								lock_operate.lock_state = LOCK_READY;
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
									if(Get_Admin_id_Number()==4)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Beep_Delete_ID_Tone();
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
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
							//	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_ADD_SHORT:
							case KEY_ADD_LONG:
								if(lock_operate.id>=99)
									lock_operate.id=95;
								id = Find_Next_Admin_Null_ID_Add(lock_operate.id);		
								if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()==4)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Beep_Delete_ID_Tone();
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
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
//								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_OK_SHORT:
							case KEY_OK_LONG:
//								fifo_clear(&touch_key_fifo);
//								lock_operate.lock_state = WAIT_PASSWORD_ONE;
//								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
//								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
							
							
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
										Beep_Null_Warm();
										lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
									}
								}
								else
								{
									gOperateBit =0;
									Beep_Null_Warm();
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
						if((lock_operate.lock_state!=LOCK_IDLE)&&(lock_operate.lock_state!=LOCK_READY)&&(lock_operate.lock_state!=WAIT_PASSWORD_ONE))
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
							Beep_Null_Warm();
						}				
						SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
					}
					else if((e.data.KeyValude==0x23) || (e.data.KeyValude==0x2a))
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
								Beep_Null_Warm();
								lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{
							gOperateBit =0;
							Beep_Null_Warm();
							lock_operate.id = Find_Next_Admin_Null_ID_Add(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
						}
				}
				else
				{}		
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
						//lock_operate.lock_state = WAIT_SELECT_USER_ID;
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
					if(len==TOUCH_KEY_PSWD_LEN)
					{				
							
							touch_key_buf[len] = '\0';
							if(Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf)==0)
							{
								Beep_Register_Sucess_Tone();
								gEventOne.event = TOUCH_KEY_EVENT;
								strcpy(gEventOne.data.Buff, touch_key_buf);
								lock_operate.lock_state = WATI_PASSWORD_TWO;
							}
							else
								Beep_Register_Fail_Warm();
							fifo_clear(&touch_key_fifo);
					}
					else if(len>TOUCH_KEY_PSWD_LEN)
					{
						Beep_Register_Sucess_Tone();
						fifo_clear(&touch_key_fifo);
					}
					else
					{
						if((e.data.KeyValude=='*')||(e.data.KeyValude=='#'))
						{
							fifo_clear(&touch_key_fifo);
							Beep_Register_Sucess_Tone();
						}
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					
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
					if(len==TOUCH_KEY_PSWD_LEN)
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
							Beep_Register_Sucess_Tone();
							if((lock_operate.id>=96) && (lock_operate.id<100))
							{
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
								id = Find_Next_Admin_Null_ID_Add(lock_operate.id);
							}
							else
							{
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								id = Find_Next_User_Null_ID_Add(lock_operate.id);
							}
							gOperateBit =0;
							
							if(id!=-1)
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								
							}
							else
							{
								lock_operate.id = 100; 
								
								SegDisplayCode = GetDisplayCodeFU();
							}
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );	
						}
						else
						{
							lock_operate.lock_state = WAIT_PASSWORD_ONE;
							Beep_Register_Fail_Warm(); 
							//toddo
						}
						memset(&gEventOne, 0, sizeof(EventDataTypeDef));
						fifo_clear(&touch_key_fifo);
					}
					else
					{
						if((e.data.KeyValude=='*')||(e.data.KeyValude=='#'))
						{
							lock_operate.lock_state = WAIT_PASSWORD_ONE;
							fifo_clear(&touch_key_fifo);
							Beep_Register_Fail_Warm(); 
						}
					}
			  }
				else if(e.event==RFID_CARD_EVENT)
				{
					//todo 
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
					if((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))
					{
							
							touch_key_buf[len] = '\0';
							if(0 !=Compare_To_Flash_Admin_id(TOUCH_PSWD, (char*)touch_key_buf))
							{
								Flash_Comare_Sucess_Warm();
								if(lock_operate.lock_action == DELETE_USER)
								{
									
									lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
									SegDisplayCode = Lock_Enter_Wait_Delete_ID();

								}
								else if(lock_operate.lock_action == DELETE_ADMIN)
								{
									lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
									SegDisplayCode = Lock_Enter_Wait_Delete_ID();
								}
								else if(lock_operate.lock_action == ADD_USER)
								{
									gOperateBit =0;
									lock_operate.id = Find_Next_User_Null_ID_Add(0);  
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									lock_operate.lock_state = WAIT_SELECT_USER_ID;
								}
								else if(lock_operate.lock_action == ADD_ADMIN)
								{
									gOperateBit =0;
									lock_operate.id = Find_Next_Admin_Null_ID_Add(0);  
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
								}
								fifo_clear(&touch_key_fifo);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//	
							}
						
					}
					
				}
				else if(e.event==RFID_CARD_EVENT)
				{
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
					if((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))
					{	
						touch_key_buf[len] = '\0';
						id = Compare_To_Flash_id(TOUCH_PSWD, (char*)touch_key_buf);
						if(id !=0)
						{
							fifo_clear(&touch_key_fifo);
							Delect_Index((uint8_t) id);
							Beep_Delete_ID_Tone();
							Delete_Mode_Temp = DELETE_USER_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}		
					}
					
				}
				else if(e.event==RFID_CARD_EVENT)
				{}
					
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
					if((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))
					{
						
						touch_key_buf[len] = '\0';
						id = Compare_To_Flash_Admin_id(TOUCH_PSWD, (char*)touch_key_buf);
						if(id !=0)
						{
							Beep_Delete_ID_Tone();
							fifo_clear(&touch_key_fifo);
							Delect_Index((uint8_t) id);
							Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
							lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
							SegDisplayCode = GetDisplayCodeFP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
						}		
					}
					
				}
				else if(e.event==RFID_CARD_EVENT)
				{}
					
				break;
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

static uint16_t GetDisplayCodeActive(void)
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



