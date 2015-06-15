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

#define SLEEP_TIMEOUT 50000/2   /* 定时器计时周期为 2ms */

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_INIT,&lock_infor,0,0,0};
struct node_struct_t process_event_scan_node;



static void process_event(void);
uint32_t SleepTime_End = 0; 

static uint16_t GetDisplayCodeNull(void);
static uint16_t GetDisplayCodeAD(void);
static uint16_t GetDisplayCodeFP(void);
static uint16_t GetDisplayCodeActive(void);
static uint16_t GetDisplayCodeNum(uint8_t num);
static uint16_t GetDisplayCodeFU(void);
static uint16_t GetDisplayCodeAL(void);

void Process_Event_Task_Register(void)
{
		lock_operate.lock_state = LOCK_INIT;
		lock_operate.user_num = 0;
		lock_operate.admin_num = 0;
		lklt_insert(&process_event_scan_node, process_event, NULL, 10/5);
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
		printf("Init lock_state: LOCK_INIT\r\n");
}

void Lock_EnterIdle(void)
{
		lock_operate.lock_state = LOCK_IDLE;
		fifo_clear(&touch_key_fifo);
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);//turn off SEG8_LED
		Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);  //turn off all led
		printf("enter LOCK_IDLE\r\n");
}
void Lock_EnterReady(void)
{
	uint16_t SegDisplayCode;
	
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
	printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
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
	id  = Find_Next_User_ID_Dec(96);	
	if(id!=-1)
	{
		lock_operate.id = id;
		code = GetDisplayCodeNum(id);
	}
	else
		code = Lock_Enter_DELETE_USER_BY_FP();
	return code;
}

uint16_t Lock_Enter_DELETE_USER_ALL(void)
{
	uint16_t code;
	
	lock_operate.lock_state = DELETE_USER_ALL;
	code = GetDisplayCodeAL();
	return code;
}
static void process_event(void)
{
	int8_t id;
	uint32_t time;
	uint16_t SegDisplayCode;
	Hal_EventTypedef e; 
	
		time= GetSystemTime();
	
		if((lock_operate.lock_state!=LOCK_IDLE)&&(time >= SleepTime_End))
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                		
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
					//	 Hal_Beep_Blink (2, 50, 50);  //需要看效果
					}
					lock_operate.lock_state = LOCK_READY;
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
					printf("-s LOCK_INIT -e EVENT_NONE -a Lock_READY\r\n");
			}
			break;
			
			case 	LOCK_IDLE:
				if((e.event==BUTTON_KEY_EVENT) || (e.event==TOUCH_KEY_EVENT))
				{
					Lock_EnterReady();
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
							printf("-s LOCK_READY -e KEY_CANCEL_SHORT -a LOCK_IDLE\r\n");
							break;
						
						case KEY_DEL_SHORT:
							if(Get_User_id_Number()>0)
							{
								lock_operate.lock_action = DELETE_ID;
								if(lock_operate.plock_infor->work_mode==NORMAL)
								{
									SegDisplayCode = Lock_Enter_DELETE_USER_BY_FP();
									printf("-s LOCK_READY -e KEY_DEL_SHORT -a DELETE_USER_BY_FP\r\n");
								}
								else
								{
									lock_operate.lock_state = WAIT_AUTHENTIC;
									SegDisplayCode = GetDisplayCodeAD();
									printf("-s LOCK_READY -e KEY_DEL_SHORT -a WAIT_AUTHENTIC\r\n");
								}
								
							}
							else
							{
								SegDisplayCode = GetDisplayCodeNull();   /* un */
								Hal_Beep_Blink (2, 50, 50);  //需要看效果
								lock_operate.lock_state = LOCK_READY;  
								printf("-s LOCK_READY -e KEY_DEL_SHORT -a LOCK_READY\r\n");
							}
							break;
							
						case KEY_ADD_SHORT:
							lock_operate.lock_action = ADD_ID;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_User_Null_ID(0);  
								lock_operate.lock_state = WAIT_SELECT_USER_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								printf("-s LOCK_READY -e KEY_ADD_SHORT -a WAIT_SELECT_USER_ID -id :%d\r\n", lock_operate.id);
								
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.lock_state = WAIT_AUTHENTIC;
								printf("-s LOCK_READY -e KEY_ADD_SHORT -a WAIT_AUTHENTIC\r\n");
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
							else 
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
								lock_operate.id = Find_Next_Admin_Null_ID(95);
								lock_operate.lock_state = WAIT_AUTHENTIC;
								SegDisplayCode = GetDisplayCodeAD();
							}
							else
							{
								lock_operate.id = Find_Next_Admin_Null_ID(95);
								lock_operate.lock_state = WATI_SELECT_ADMIN_ID;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							}
							break;		
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							lock_operate.lock_action = ADD_ID;	
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								lock_operate.id = Find_Next_User_Null_ID(0);
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								printf("-s LOCK_READY -e KEY_OK -a WAIT_PASSWORD_ONE\r\n");
							}	
							else
							{
								SegDisplayCode = GetDisplayCodeAD();
								lock_operate.id = 0;
								lock_operate.lock_state = WAIT_AUTHENTIC;
								printf("-s LOCK_READY -e KEY_OK -a WAIT_AUTHENTIC\r\n");
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
						if(len==TOUCH_KEY_PSWD_LEN)
							Hal_Beep_Blink (2, 10, 50);  //需要看效果
						if((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))
						{
							touch_key_buf[len] = '\0';
							id = Compare_To_Flash_id((char*)touch_key_buf);
							if(id!=0)
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
							}
							else if(len>=TOUCH_KEY_PSWD_MAX_LEN)
							{
								fifo_clear(&touch_key_fifo);
								Hal_Beep_Blink (2, 10, 50);  //失败报警
							}
						}
							
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					
				}
				break;
				
				
/*
#define 	KEY_CANCEL_SHORT	 1
#define 	KEY_DEL_SHORT 		 2
#define 	KEY_OK_SHORT  		 4
#define 	KEY_INIT_SHORT 		 8
#define 	KEY_ADD_SHORT   	0x10

#define 	KEY_CANCEL_LONG	 KEY_CANCEL_SHORT |0x80
#define 	KEY_DEL_LONG 		 KEY_DEL_SHORT |0x80
#define 	KEY_OK_LONG  		 KEY_OK_SHORT |0x80
#define 	KEY_INIT_LONG 	 KEY_INIT_SHORT |0x80
#define 	KEY_ADD_LONG   	 KEY_ADD_SHORT |0x80
*/
			case WAIT_SELECT_USER_ID:
				if(e.event==BUTTON_KEY_EVENT) 
				{
						switch (e.data.KeyValude)
						{
							case KEY_CANCEL_SHORT:
							case KEY_CANCEL_LONG:
								lock_operate.lock_state = LOCK_READY;
								SegDisplayCode = GetDisplayCodeActive();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
								printf("-s WAIT_SELECT_USER_ID -e KEY_CANCEL_LONG -a LOCK_READY\r\n");
								break;
							case KEY_DEL_SHORT:
							case KEY_DEL_LONG:

								if(lock_operate.lock_action == ADD_ID)
									id = Find_Next_User_Null_ID_Dec(lock_operate.id);		
								else if(lock_operate.lock_action == DELETE_ID)
									id = Find_Next_User_ID_Dec(lock_operate.id);	
								if(id==-1) //无数据 delete 操作
								{
									if(Get_User_id_Number())
									{
										lock_operate.id = Find_Next_User_ID_Dec(96);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}									
								}
								else if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()==95)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}
									else
									{
										lock_operate.id = Find_Next_User_ID_Dec(96);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
								}
								else
								{
									
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									printf("-s WAIT_SELECT_USER_ID -e KEY_DEL_SHORT -id %d\r\n",lock_operate.id);
								}
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_ADD_SHORT:
							case KEY_ADD_LONG:
								if(lock_operate.id>=95)
									lock_operate.id=0;
								if(lock_operate.lock_action == ADD_ID)
									id = Find_Next_User_Null_ID(lock_operate.id);		
								else if(lock_operate.lock_action == DELETE_ID)
									id = Find_Next_User_ID(lock_operate.id);	
								if(id==-1) //无数据 delete 操作
								{
									if(Get_User_id_Number())
									{
										lock_operate.id = Find_Next_User_ID(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}									
								}
								else if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()==95)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}
									else
									{
										lock_operate.id = Find_Next_User_Null_ID(0);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
										
									}
								}
								else
								{
									lock_operate.id = id;
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									printf("-s WAIT_SELECT_USER_ID -e KEY_ADD -id %d\r\n",lock_operate.id);
								}
								printf("-s WAIT_SELECT_USER_ID -e KEY_ADD -id %d\r\n",lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_OK_SHORT:
							case KEY_OK_LONG:
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
								printf("-s WAIT_SELECT_USER_ID -e KEY_OK -id %d \r\n",lock_operate.id);
								break;
							case KEY_INIT_SHORT:
							case KEY_INIT_LONG:
								break;
							default :
								break;
						}
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
					
				}
				
				break;
			case WATI_SELECT_ADMIN_ID:
					if(e.event==BUTTON_KEY_EVENT) 
				{
						switch (e.data.KeyValude)
						{
							case KEY_CANCEL_SHORT:
							case KEY_CANCEL_LONG:
								lock_operate.lock_state = LOCK_READY;
								SegDisplayCode = GetDisplayCodeActive();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
								break;
							case KEY_DEL_SHORT:
							case KEY_DEL_LONG:
								if(lock_operate.id<=96)
									lock_operate.id=100;		
								if(lock_operate.lock_action == ADD_ID)
									id = Find_Next_Admin_Null_ID_Dec(lock_operate.id);		
								else if(lock_operate.lock_action == DELETE_ID)
									id = Find_Next_Admin_ID_Dec(lock_operate.id);	
								if(id==-1) //无数据 delete 操作
								{
									if(Get_Admin_id_Number())
									{
										lock_operate.id = Find_Next_Admin_ID_Dec(100);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}									
								}
								else if(id==-2) // add 操作，数据已满
								{
									if(Get_Admin_id_Number()==4)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
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
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
								break;
							case KEY_ADD_SHORT:
							case KEY_ADD_LONG:
								if(lock_operate.id>=99)
									lock_operate.id=95;
								if(lock_operate.lock_action == ADD_ID)
									id = Find_Next_Admin_Null_ID(lock_operate.id);		
								else if(lock_operate.lock_action == DELETE_ID)
									id = Find_Next_Admin_ID(lock_operate.id);	
								if(id==-1) //无数据 delete 操作
								{
									if(Get_User_id_Number())
									{
										lock_operate.id = Find_Next_Admin_ID(95);
										SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									}
									else
									{
										SegDisplayCode = GetDisplayCodeNull();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}									
								}
								else if(id==-2) // add 操作，数据已满
								{
									if(Get_User_id_Number()==4)
									{
										SegDisplayCode = GetDisplayCodeFU();
										Hal_Beep_Blink (2, 50, 50);  //需要看效果
										
										Lock_EnterIdle();
										lock_operate.lock_state = LOCK_IDLE;
									}
									else
									{
//										lock_operate.id = Find_Next_Null_ID(95);
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
							case KEY_OK_LONG:
								lock_operate.lock_state = WAIT_PASSWORD_ONE;
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
								break;
							case KEY_INIT_SHORT:
							case KEY_INIT_LONG:
								break;
							default :
								break;
						}
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//
				}
				else if(e.event==TOUCH_KEY_EVENT)
				{
				}
				
				break;

			case WAIT_PASSWORD_ONE:
			case WATI_PASSWORD_TWO:
			case WAIT_AUTHENTIC:
			case DELETE_USER_BY_FP:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							Lock_EnterReady();
							break;
						case KEY_DEL_SHORT:
						  SegDisplayCode = Lock_Enter_DELETE_USER_ALL();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
							break;
						case KEY_ADD_SHORT:
							SegDisplayCode =Lock_Enter_DELETE_USER_ID();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							break;
						case KEY_OK_SHORT:
							
							break;
						case KEY_INIT_SHORT:
							break;

							default:
								break;
					}
				}
			case DELETE_USER_ALL:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							Lock_EnterReady();
							break;
						case KEY_DEL_SHORT:
							SegDisplayCode =Lock_Enter_DELETE_USER_ID();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
							break;
						case KEY_ADD_SHORT:
							SegDisplayCode =Lock_Enter_DELETE_USER_BY_FP();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							break;
						case KEY_OK_SHORT:
							break;
						case KEY_INIT_SHORT:
							break;
							default:
								break;
					}
				}				
			case DELETE_USER_ID:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							Lock_EnterReady();
							break;
						case KEY_DEL_SHORT:
							id  = Find_Next_User_ID_Dec(lock_operate.id);	
							if(id!=-1)
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(id);
							}
							else
								SegDisplayCode = Lock_Enter_DELETE_USER_BY_FP();
							
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							printf("-s LOCK_IDLE -e BUTTON_KEY_EVENT -a Lock_READY\r\n");
							break;
						case KEY_ADD_SHORT:
							id  = Find_Next_User_ID(lock_operate.id);	
							if(id!=-1)
							{
								lock_operate.id = id;
								SegDisplayCode = GetDisplayCodeNum(id);
							}
							else
								SegDisplayCode = Lock_Enter_DELETE_USER_ALL();
							
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							break;
						case KEY_OK_SHORT:
							break;
						case KEY_INIT_SHORT:
							break;

							default:
								break;
					}
				}
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

static uint16_t GetDisplayCodeAL(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[17];
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



