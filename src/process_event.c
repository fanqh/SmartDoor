#include "process_event.h"
#include "Link_list.h"
#include "string.h"
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
#include "lock_key.h"
#include "rf_vol_judge.h"
#include "uart.h"
#include "adc.h"


#define UNLOCK_TIMEOUT  6
#define DEBUG_  1

lock_operate_srtuct_t lock_operate = {ACTION_NONE,LOCK_READY,&lock_infor,0,0,0,0xffff,&finger_state};
struct node_struct_t process_event_scan_node;
static uint32_t MotorEndTime = 0;
 uint32_t SleepTime_End = 0; 
static uint32_t Lock_Restrict_Time=0;	
static uint32_t PW_Err_Count = 0;
char gpswdOne[TOUCH_KEY_PSWD_LEN+1];
Hal_EventTypedef gEventOne;
static LOCK_STATE Delete_Mode_Temp = DELETE_USER_BY_FP;
static uint8_t gOperateBit = 0;   //0  digits  1 decimal  2 warm 
Motor_State_t motor_state = MOTOR_NONE;

uint8_t Unlock_Warm_Flag=0;
uint8_t is_Err_Warm_Flag = 0;

static uint16_t bug = 0;
extern uint8_t vol_low_warm_flag;


static uint16_t GetDisplayCodeAD(void);
static uint16_t GetDisplayCodeFP(void);
static uint16_t GetDisplayCodeNum(uint8_t num);
static uint16_t GetDisplayCodeFU(void);
static uint16_t GetDisplayCodeAL(void);
static uint16_t GetDisplayCodeFE(void);
static void Lock_Enter_Unlock_Warm(void);
static uint16_t GetDisplayCodeCL(void);
static void Lock_Enter_Err(void);
static void RTC_Config(void);
static void ReadyState_Compare_Pass_Display(uint8_t id);


static const char* lock_state_str[]=
{
	"LOCK_INIT",
	"LOCK_IDLE" ,
	"LOCK_UNLOCK_WARM",
	"LOCK_READY",
	"WAIT_SELECT_USER_ID",
	"WATI_SELECT_ADMIN_ID",
	"WAIT_PASSWORD_ONE",
	"WATI_PASSWORD_TWO",
	"WAIT_AUTHENTIC",
	"WAIT_SELECT_DELETE_MODE",
//	WAIT_DELETE_ADMIN_MODE,
	"DELETE_USER_BY_FP",
	"DELETE_USER_ALL",
	"DELETE_USER_ID",
	"DELETE_ADMIN_BY_FP",
	"DELETE_ADMIN_ALL",
	"DELETE_ADMIN_ID",
	"ADD_ID_OK",
	"DELETE_ID_OK",
	"LOCK_OPEN_CLOSE",
	"LOCK_OPEN",
	"LOCK_CLOSE",
	"LOCK_ERR",
	"LOCK_OPEN_NORMAL",
	"LOCK_GET_ID_NUM"
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



void PASSWD_COMPARE_ERR(void)				
{
//	Hal_Beep_Clear();
	Beep_PWM_TimeBase_config(760); 
	Beep_PWM_config(360);
	Beep_ON();
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_RED_ON_VALUE);
	delay_ms(200);
	Beep_OFF();
	Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_RED_ON_VALUE);
	
	delay_ms(200);
	Hal_Beep_Clear();
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_BLUE_ALL_ON_VALUE);
	//Hal_Beep_Blink (2, 50,50); 
}

uint16_t GetDisplayCodeNull(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[19];   /* u */
	code = (code<<8) | LEDDisplayCode[20]; /*  displya u n*/				
	return code;
}
uint16_t GetDisplayCodeCL(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[12];   /* L */
	code = (code<<8) | LEDDisplayCode[17]; /*  displya CL*/				
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
	
	code = LEDDisplayCode[15];
	code = (code<<8) | LEDDisplayCode[18];/*  FP */
	return code;	
}

 uint16_t GetDisplayCodeActive(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[16];
	code = (code<<8) | LEDDisplayCode[16];/*  -- */
	return code;	
}

uint16_t GetDisplayCodeBatteryLowlMode(void)
{
	uint16_t code;
	
	code = LEDDisplayCode[17];
	code = (code<<8) | LEDDisplayCode[18];/*  LP */
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
	
	code = LEDDisplayCode[15];
	code = (code<<8) | LEDDisplayCode[20];/*  FU */
	return code;	
}

static uint16_t GetDisplayCodeFE(void)  /*错误密码超出系统自锁*/
{
		uint16_t code;
	
	code = LEDDisplayCode[15];
	code = (code<<8) | LEDDisplayCode[14];/*  FE */
	return code;	
}
void Delete_ID_Flash_Once(uint8_t id)
{
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeNum(id)); 
	delay_ms(300);
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, GetDisplayCodeNum(id)); 
}	


// A = 120-1; B = 300-1;   interval = 870ms
//A = 120-1; B = 500;    interval = 1.46s
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
    RTC_InitStructure.RTC_AsynchPrediv = 128-1;  //120/40k = 3ms
    RTC_InitStructure.RTC_SynchPrediv = 300-1;    //200
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
	RTC_AlarmSubSecondConfig(RTC_Alarm_A, 1, RTC_AlarmSubSecondMask_None);  // 1 / (40K/(PREDIV_S-1)*(PREDIV_A-1))
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
#if 0
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
#endif
	

void Process_Event_Task_Register(void)
{
		lock_operate.user_num = 0;
		lock_operate.admin_num = 0;
		lklt_insert(&process_event_scan_node, process_event, NULL, 1*TRAV_INTERVAL);//TRAV_INTERVAL  10ms
		SleepTime_End = GetSystemTime() + SLEEP_TIMEOUT;
}


uint16_t Lock_EnterReady(void)
{	
	if(is_finger_ok) 
		Match_finger();
	
	gOperateBit =0;
	fifo_clear(&touch_key_fifo);
	lock_operate.lock_state = LOCK_READY;

	return GetDisplayCodeActive();
}
static uint16_t Lock_Enter_DELETE_USER_BY_FP(void)
{
	uint16_t code;
	if(is_finger_ok) 	
		Match_finger();

	lock_operate.lock_state = DELETE_USER_BY_FP;
	code = GetDisplayCodeFP();
	return code;
}





static uint16_t Lock_Enter_DELETE_ADMIN_BY_FP(void)
{
	uint16_t code;
	if(is_finger_ok) 
		Match_finger();
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
static void Lock_Enter_Passwd_One(void)
{
	if(is_finger_ok) 	
		Finger_Regist_CMD2();
	fifo_clear(&touch_key_fifo);
	lock_operate.lock_state = WAIT_PASSWORD_ONE;

}

static uint16_t Lock_Enter_Authntic(void)
{
	if(is_finger_ok) 
		Match_finger();
	fifo_clear(&touch_key_fifo);
	lock_operate.lock_state = WAIT_AUTHENTIC;
	return GetDisplayCodeAD();	
}

uint16_t Lock_EnterIdle(void)
{
	uint32_t retry = 0;

//	printf("idle11111111\r\n");
	while(mpr121_get_irq_status()==0)
	{
		delay_us(1);
		retry++;
		if(retry>1000)
		{
			printf("mpr121 is hold\r\n");
			return 0;
		}
	}
//	printf("idle.......\r\n");
	mpr121_enter_standby();
	Finger_RF_LDO_Disable();;	
		
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
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
	__disable_irq();
//	if(mpr121_get_irq_status()!=0)
//		PWR_EnterSTANDBYMode(); 
	retry = 0;
	while((GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)==0)&&(mpr121_get_irq_status()==0)&&(retry<1000*1000))
	{
		delay_us(1);
		retry++;
	}
	if((mpr121_get_irq_status()==0)&&(GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)!=0))
		return 0xffff;
	PWR_EnterSTANDBYMode(); 
	 return 0xffff;
}


uint16_t Lock_EnterIdle1(void)
{
	uint16_t retry = 0;
	uint16_t err_Timecount = 0;

	while(mpr121_get_irq_status()==0)
	{
//			delay_us(1);
		retry++;
		if(retry>5000)
			return 0;
	}
#if 1
	err_Timecount = GetLockFlag(ERROR_STATE_TIMECOUNT_ADDR);
	if(err_Timecount!=0xffff)
	{
		err_Timecount ++;
		WriteLockFlag(ERROR_STATE_TIMECOUNT_ADDR, err_Timecount);
		printf("err_timecount = %d\r\n",err_Timecount);
	}
	if(Get_Lock_Pin_State()==0)
	{
		uint32_t Lock_TimeCount;
		if((Lock_TimeCount = GetLockFlag(FLASH_LOCK_FLAG_ADDR))==0xffff)
			Lock_TimeCount=0;
		Lock_TimeCount ++;
		printf("timeCount1 = %d\r\n",Lock_TimeCount);
		if(Lock_TimeCount==UNLOCK_TIMEOUT)
		{
			Unlock_Warm_Flag = 1;
			Lock_Enter_Unlock_Warm();
			Lock_TimeCount++;
			WriteLockFlag(FLASH_LOCK_FLAG_ADDR, Lock_TimeCount);
//			EreaseAddrPage(FLASH_LOCK_FLAG_PAGE*FLASH_PAGE_SIZE);
			return 1;
		}
		else
		{	
			if(Lock_TimeCount<(UNLOCK_TIMEOUT+1))
			{
				printf("write %d to flash\r\n",Lock_TimeCount);
				WriteLockFlag(FLASH_LOCK_FLAG_ADDR, Lock_TimeCount);
			}
		}
	}
	else if(GetLockFlag(FLASH_LOCK_FLAG_ADDR)!=0xffff)
	{
		printf("lock release\r\n");
		EreaseAddrPage(FLASH_LOCK_FLAG_ADDR);
	}
#endif
	Finger_RF_LDO_Disable();
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
	__disable_irq();
	if(mpr121_get_irq_status()!=0)
		PWR_EnterSTANDBYMode(); 
	
	 return 0xffff;
}

uint16_t Lock_EnterIdle2(void)
{
	uint32_t retry = 0;

	mpr121_enter_standby();
	Finger_RF_LDO_Disable();;	
		
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
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
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeNull() );
	Beep_Null_Warm_Block();
	Lock_EnterIdle(); 	
}


void Lock_Err_Three_Times_Warm(void)
{
	uint16_t SegDisplayCode;
	
	is_Err_Warm_Flag = 1;
	printf("with in Three err times \r\n");
	Exit_Finger_Current_Operate();
//	HC595_Power_ON();
	SegDisplayCode = GetDisplayCodeFE();
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
	HC595_ShiftOut16(SER_LED_INTERFACE, LED_RED_ON_VALUE);
	
	Beep_Three_Time();
	
	Lock_EnterIdle2();
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);
	Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);
	HC595_Power_OFF();
	Lock_Restrict_Time = GetSystemTime() + 100;//3min
	lock_operate.lock_state = LOCK_ERR; 

	
	
	printf("err....state...should idle\r\n");
}
static void Lock_Enter_Unlock_Warm(void)
{
   enum wakeup_source_t mode = OTHER_WAKEUP;
	
	Init_Module(mode);
	SleepTime_End = GetSystemTime() + 5000;
	LOCK_ERR_Warm();
	printf("..........\r\n");
//	lock_operate.lock_state = LOCK_UNLOCK_WARM;
}

static void Lock_Enter_Err(void)
{
	uint16_t err_Timecount = 1;
	
	WriteLockFlag(ERROR_STATE_TIMECOUNT_ADDR, err_Timecount);
	Lock_Err_Three_Times_Warm();
}

static void Enter_NOUSER(void)
{
#if 0
	uint16_t SegDisplayCode;
	
	printf("no id was registed\r\n");
	lock_operate.id = 0;
	SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//需要确认之后的状态
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_GREEN_ON_VALUE);
	lock_operate.lock_state = LOCK_OPEN_CLOSE;	
	fifo_clear(&touch_key_fifo);
	Exit_Finger_Current_Operate();
#endif
	ReadyState_Compare_Pass_Display(0);
}

//src_id 
/*
0:touch_key, 
1: finger, 
2, RF  
3: AD  模式下
*/
static void ReadyState_CompareErrCount_Add(uint8_t src_id)
{
	printf("compare fail\r\n");
	
	if((src_id==0)||(src_id==3))
		PW_Err_Count++;
	if(PW_Err_Count>=3)
	{
		Lock_Enter_Err();
	}
	else
	{
		PASSWD_COMPARE_ERR();
//		if(src_id==1)
//			delay_ms(300);
		if(src_id!=3)
			Lock_EnterReady();
		
	}
	fifo_clear(&touch_key_fifo);
}

static void ReadyState_Compare_Pass_Display(uint8_t id)
{
	uint16_t SegDisplayCode;
	
	lock_operate.id = id;
	SegDisplayCode = GetDisplayCodeNum(lock_operate.id);	
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//需要确认之后的状态
	PASSWD_SUCESS_ON();
	lock_operate.lock_state = LOCK_OPEN_CLOSE;	
	fifo_clear(&touch_key_fifo);
	Exit_Finger_Current_Operate();	
}

static void ReadyState_Compare_ID_Judge(uint8_t id, uint8_t src_id)  //src_id 0:touch_key, 1: finger, 2, RF  3: AD  模式下
{
	if(id!=0)
	{
		printf("compare ok\r\n");
		ReadyState_Compare_Pass_Display(id);
	}
	else 
		ReadyState_CompareErrCount_Add(src_id);	
}
void Action_Delete_All_ID(void)
{
	uint16_t SegDisplayCode;
	
	Delete_All_ID();
	SegDisplayCode = GetDisplayCodeCL(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode ); 
	if(is_finger_ok) 
		Delete_All_Finger();

	PASSWD_Delete_ALL_ID();
	
	delay_ms(640);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_GREEN_ON_VALUE);
	SegDisplayCode = GetDisplayCodeNull(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode ); 
	Beep_Three_Time();
	
	Lock_EnterIdle();
}

void Action_Delete_All_User_ID(void)
{
	int8_t i = 0;
	int8_t id=0;
	uint16_t fingernum=0;
	uint16_t SegDisplayCode;
	id_infor_t  id_infor;
	uint16_t fingerid = 0;
	uint16_t count = 0;
	
	
	
 
	if(is_finger_ok) 
	{
		Get_Finger_Num(&fingernum);
		for(i=0; (i<=95) && ((id =Find_Next_User_ID_Add(id))!=-1) && (count<fingernum); i++)
		{
			Read_Select_ID(id, &id_infor);
			if(id_infor.type==FINGER_PSWD)
			{
				count ++;
				fingerid = id_infor.password[0] + id_infor.password[1]*256;
				Delelte_ONE_Finger(fingerid);
			}
		}
	}
	Delete_All_User_ID();
	SegDisplayCode = GetDisplayCodeCL(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );

	PASSWD_Delete_ALL_ID();
	
	delay_ms(640);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_GREEN_ON_VALUE);
	SegDisplayCode = GetDisplayCodeNull(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode ); 
	Beep_Three_Time();
	
	Lock_EnterIdle();	
}


void Action_Delete_All_Admin_ID(void)
{
	int8_t i = 0;
	int8_t id=0;
	uint16_t fingernum=0;
	uint16_t SegDisplayCode;
	id_infor_t  id_infor;
	uint16_t fingerid = 0;
	uint16_t count = 0;
	if(is_finger_ok) 
	{
		Get_Finger_Num(&fingernum);
		for(i=95; (i<100) && ((id =Find_Next_Admin_ID_Add(id))!=-1) && (count<fingernum); i++)
		{
			Read_Select_ID(id, &id_infor);
			if(id_infor.type==FINGER_PSWD)
			{
				count ++;
				fingerid = id_infor.password[0] + id_infor.password[1]*256;
				Delelte_ONE_Finger(fingerid);
			}
		}
	}
	Delete_All_Admin_ID();
	SegDisplayCode = GetDisplayCodeCL(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );

	PASSWD_Delete_ALL_ID();
	
	delay_ms(640);
	Hal_LED_Display_Set(HAL_LED_MODE_ON, LED_GREEN_ON_VALUE);
	SegDisplayCode = GetDisplayCodeNull(); 
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode ); 
	Beep_Three_Time();
	
	Lock_EnterIdle();	
}

uint8_t is_Motor_Moving(void)
{
	if((lock_operate.lock_state==LOCK_OPEN_CLOSE)||(lock_operate.lock_state==LOCK_OPEN)||(lock_operate.lock_state==LOCK_CLOSE)||(lock_operate.lock_state==LOCK_OPEN_NORMAL))
		return 1;
	else
		return 0;
}

uint8_t is_Motor_Moving1(void)
{
	if((lock_operate.lock_state==LOCK_OPEN_CLOSE)||(lock_operate.lock_state==LOCK_OPEN)||(lock_operate.lock_state==LOCK_CLOSE))
		return 1;
	else
		return 0;
}

static void PasswdTwoCompara_Sucess(id_infor_t id_infor)
{
	int8_t id;
	uint16_t SegDisplayCode;

	Add_One_ID(lock_operate.id, id_infor);
	PASSWD_COMPARE_OK();
	if((lock_operate.id>=96) && (lock_operate.id<100))
	{
		if(Get_Admin_id_Number()>=4)
			Lock_FU_Indication();
		else
		{
			lock_operate.lock_state = WATI_SELECT_ADMIN_ID;	
			id = Find_Next_Admin_Null_ID_Add(0);
		}
	}
	else
	{
		if(Get_User_id_Number()>=95)
			Lock_FU_Indication();
		else
		{
			lock_operate.lock_state = WAIT_SELECT_USER_ID;
			id = Find_Next_User_Null_ID_Add(0);
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
	else//如果ID依旧为空，，说明锁的状态跑错了
	{
		lock_operate.id = 100; 
		SegDisplayCode = GetDisplayCodeFU();
		ERR_UNKNOWN();  //需要看效果 
	}
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
}

static void WaitAuthentic_OK(void)
{	
	uint16_t SegDisplayCode;
	
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
	else if(lock_operate.lock_action == GET_ID_NUM)
	{
		lock_operate.lock_state = LOCK_GET_ID_NUM;
		SegDisplayCode = GetDisplayCodeNum(Get_id_Number());
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
	}
	fifo_clear(&touch_key_fifo);
	if((lock_operate.lock_state!=LOCK_READY)&&(lock_operate.lock_state != LOCK_GET_ID_NUM))
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//	
	else
		Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//							
}
void Wait_Select_Delete_Mode(LOCK_STATE state)
{
	if((state<DELETE_USER_BY_FP)||(state>DELETE_ADMIN_ID))
		return;
	fifo_clear(&touch_key_fifo);
	Delete_Mode_Temp = state;
	lock_operate.lock_state = WAIT_SELECT_DELETE_MODE;
	Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, GetDisplayCodeFP());//
}



void process_event(void)
{
	int8_t id;
	uint32_t time;
	uint16_t SegDisplayCode;
	Hal_EventTypedef e; 
	
	time= GetSystemTime();
	e = GetEvent();

	if((lock_operate.lock_state!=LOCK_IDLE)&&(time >= SleepTime_End)&&(lock_operate.lock_state!=LOCK_ERR)&&(!is_Motor_Moving1()))
	{
			printf("idly time = %d,,endtime = %d\r\n", time,SleepTime_End);
			Lock_EnterIdle();		
	}		
	if((lock_operate.lock_state==LOCK_ERR)&&(time>Lock_Restrict_Time))
			Lock_EnterIdle();
	if((Unlock_Warm_Flag==1)&&(Get_Lock_Pin_State()!=0))
		Lock_EnterIdle();
//	printf("%d\r\n",lock_operate.lock_state);
	
  if((e.event==EVENT_NONE)&&(!is_Motor_Moving()))//需要替换掉  20151213
		return;
	else
	{
		if(lock_operate.lock_state!=LOCK_OPEN_NORMAL)
			SleepTime_End = time + SLEEP_TIMEOUT;
		if(e.event==TOUCH_KEY_EVENT)
		{  
			if(!(is_Motor_Moving()||(lock_operate.lock_state==LOCK_OPEN_NORMAL)))
				Hal_LED_Display_Set(HAL_LED_MODE_BLINK, GetLedVlaueFromKey(e.data.KeyValude));
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
			
			printf("-e: %s, Prestate: %s, Touch_Value: %c " , "TOUCH_KEY_EVENT", lock_state_str[lock_operate.lock_state], (char)(e.data.KeyValude));
			snprintf(str, Get_fifo_size(&touch_key_fifo)+1, (char*)(touch_key_buf));
			printf("len= %d, touch_fifo: %s\r\n\r\n", Get_fifo_size(&touch_key_fifo), str);
		}
		else if(e.event==RFID_CARD_EVENT)
		{
			printf("-e: %s, Prestate: %s\r\n" , "RFID_CARD_EVENT", lock_state_str[lock_operate.lock_state]);
		}
		else if(e.event==FINGER_EVENT)
		{
			printf("-e: %s, Prestate: %s\r\n" , "FINGER_EVENT", lock_state_str[lock_operate.lock_state]);
		}



#endif		
		
		
				
		switch(lock_operate.lock_state)
		{		
			case 	LOCK_IDLE:
				Lock_EnterReady();
				break;
			case 	LOCK_INIT:
//				Lock_EnterIdle();
				break;
			case LOCK_UNLOCK_WARM:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch (e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Lock_EnterIdle();
							break;	
						default:
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_EnterReady() );	
							break;
					}
				}
				else if(e.event!=EVENT_NONE)
				{
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_EnterReady() );		
				}
				break;
			case LOCK_ERR:
				if(e.event!=EVENT_NONE)
				{
					is_Err_Warm_Flag = 1;
//					if(GetSystemTime()-bug<1000)
//						 return;
//					bug = GetSystemTime();
//					ClearAllEvent();
//					printf("e.event = %d\r\n",e.event);
//					is_Err_Warm_Flag = 1;
//					HC595_Power_ON();
//					SegDisplayCode = GetDisplayCodeFE();
//					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
//					HC595_ShiftOut16(SER_LED_INTERFACE,LED_RED_ON_VALUE);
//					
//					Beep_Three_Time();
//					Hal_SEG_LED_Display_Set(HAL_LED_MODE_OFF, 0xffff);/* 显示地位在后 */
//					Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);
//					HC595_Power_OFF();
//					is_Err_Warm_Flag = 0;
//					ClearAllEvent();
					
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
							printf("Button_Cancle_Flag = %d\r\n", Button_Cancle_Flag);
							if(Button_Cancle_Flag!=1)
							{
								printf("idle............");
								Lock_EnterIdle();
							}
							break;
						
						case KEY_DEL_SHORT:
							lock_operate.lock_action = DELETE_USER;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								if(Get_User_id_Number()>0)
								{
									Exit_Finger_Current_Operate();
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
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_Authntic() );
							}
							break;
							
						case KEY_ADD_SHORT:
							lock_operate.lock_action = ADD_USER;
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								if(Get_User_id_Number()<95)
								{
									Exit_Finger_Current_Operate();
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
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_Authntic() );
							}	
														
							break;
						case KEY_DEL_LONG:
							lock_operate.lock_action = DELETE_ADMIN;
							if(Get_Admin_id_Number()!=0)  //大于0， 肯定在SECURITY mode下
							{
								lock_operate.plock_infor->work_mode=SECURITY;
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_Authntic() );
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
								lock_operate.id = Find_Next_Admin_Null_ID_Add(95);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_Authntic() );
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
									Exit_Finger_Current_Operate();
								}
								Exit_Finger_Current_Operate();
							}
							break;		
							
						case KEY_OK_SHORT:
						case KEY_OK_LONG:
							lock_operate.lock_action = GET_ID_NUM;	
						    fifo_clear(&touch_key_fifo);
							if(lock_operate.plock_infor->work_mode==NORMAL)
							{
								
								lock_operate.lock_state = LOCK_GET_ID_NUM;
								SegDisplayCode = GetDisplayCodeNum(Get_id_Number());
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
								Exit_Finger_Current_Operate();
							}	
							else
							{
								lock_operate.id = 0;
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_Authntic() );
							}		

							break;
							
						case KEY_INIT_LONG:
							lock_operate.lock_action = DELETE_ALL;
							Action_Delete_All_ID();
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
					if((len==1)&&(e.data.KeyValude=='#'))
					{
						fifo_clear(&touch_key_fifo);
						break;
					}
					if(e.data.KeyValude=='*')
					{
						
						if(len>1)
						{
							fifo_clear(&touch_key_fifo);
						}
						else
							Lock_EnterIdle();
//							fifo_clear(&touch_key_fifo);
					}
					else if((len>=TOUCH_KEY_PSWD_MAX_LEN)||(e.data.KeyValude=='#'))
					{
						if(Get_id_Number()==0)//没有用户 
						{
							Enter_NOUSER();
							
						}
						else
						{
							if(len>=TOUCH_KEY_PSWD_MIN_LEN)
							{
								if(e.data.KeyValude=='#')
									len = len -1;
								printf("len = %d\r\n", len);
								touch_key_buf[len] = '\0';
								id = Compare_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1,0x03);
								ReadyState_Compare_ID_Judge(id,0);
							}
							else
								ReadyState_CompareErrCount_Add(0);
						}
					}	
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					if(Get_id_Number()==0)//没有用户
					{
						Enter_NOUSER();
					}
					else
					{
						id = Compare_To_Flash_id(RFID_PSWD, RFID_CARD_NUM_LEN, (char*)e.data.Buff,1,0x03);
						ReadyState_Compare_ID_Judge(id,2);
					}
					
				}
				else if(e.event==FINGER_EVENT)
				{	
					if(e.data.Buff[0]==MATCH_CMD)//&& (e.data.Buff[0]==ACK_SUCCESS)
					{
						if(Get_id_Number()==0)//没有用户
						{
							Enter_NOUSER();	
						}
						else
						{
							uint16_t finger_num;
							
							finger_num = e.data.Buff[2] + e.data.Buff[3]*256;
							if(finger_num!=0)//比对成功
							{
								int8_t id;	
								id = Get_Finger_From_InterIndex(finger_num);
								if(id>0)
								{   
									ReadyState_Compare_Pass_Display(id);
								}
								else  //错误状态，指纹头里注册过，但是在flash index中找不到
								{
									printf("err,internal flash cannt find this id, but in the finger can find it\r\n");
									Delelte_ONE_Finger(finger_num);
									Lock_EnterIdle();
								}
							}
							else //比对失败
								ReadyState_CompareErrCount_Add(1);
						}
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
									ERR_UNKNOWN();  //需要看效果 
									Lock_EnterReady();
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
								ERR_UNKNOWN();  //需要看效果
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
									ERR_UNKNOWN();  //需要看效果
									Lock_EnterReady();
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
								ERR_UNKNOWN();  //需要看效果
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
									ERR_UNKNOWN();  //需要看效果
									Lock_EnterReady();
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
								ERR_UNKNOWN();  //需要看效果
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
									ERR_UNKNOWN();  //需要看效果
									Lock_EnterReady();
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
								ERR_UNKNOWN();  //需要看效果
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
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_USER_BY_FP() );  
						}
						else if(Delete_Mode_Temp == DELETE_USER_ALL)
						{
							Action_Delete_All_User_ID();
						}
						else if(Delete_Mode_Temp == DELETE_USER_ID)
						{
							if((lock_operate.id>id_l)&&(lock_operate.id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									PASSWD_COMPARE_OK(); 
									Delect_One_ID(lock_operate.id);
									if(Get_User_id_Number()==0)
										Lock_NULL_Indication();
									SegDisplayCode = GetDisplayCodeFP();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
								}
								else 
								{
									gOperateBit =0;
									BIT_MORE_TWO_WARM();          
									lock_operate.id = pFun(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								
							}
							else
							{ //错误状态
								gOperateBit =0;
								ERR_UNKNOWN();
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
							Action_Delete_All_Admin_ID();
						}
						else if(Delete_Mode_Temp == DELETE_ADMIN_ID)
						{
							if((lock_operate.id>id_l)&&(lock_operate.id<=id_h))
							{
								if(pFun(lock_operate.id-1)==lock_operate.id)
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									PASSWD_COMPARE_OK();  
									Delect_One_ID(lock_operate.id);
									if(Get_Admin_id_Number()==0)
									{
										Set_Work_Mode(NORMAL);
										Lock_NULL_Indication();
									}
									SegDisplayCode = GetDisplayCodeFP();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
								}
								else 
								{
									gOperateBit =0;
									BIT_MORE_TWO_WARM(); 
									lock_operate.id = pFun(0);
									SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								}
								
							}
							else
							{//Err错误状态
								gOperateBit =0;
								ERR_UNKNOWN();
								lock_operate.id = pFun(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
								printf("err system\r\n");
							}
						}
						else
						{//Err
							ERR_UNKNOWN(); 
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
				fifo_clear(&touch_key_fifo);
				if((Delete_Mode_Temp == DELETE_ADMIN_ID)||(Delete_Mode_Temp ==DELETE_USER_ID))
				{
				
					if((e.data.KeyValude>='0')&&(e.data.KeyValude<='9'))
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
						if((lock_operate.id>id_l)&&(lock_operate.id<=id_h))
						{
							if(pFun(lock_operate.id-1)==lock_operate.id)
							{
								Delect_One_ID(lock_operate.id);
								PASSWD_COMPARE_OK(); 
								
								if(Delete_Mode_Temp==DELETE_ADMIN_ID)
								{
									Delete_Mode_Temp = DELETE_ADMIN_BY_FP;
									if(Get_Admin_id_Number()==0)
										Lock_NULL_Indication();
								}
								else
								{
									Delete_Mode_Temp = DELETE_USER_BY_FP;
									if(Get_User_id_Number()==0)
										Lock_NULL_Indication();
								}

								SegDisplayCode = GetDisplayCodeFP();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 	
							}
							else //此ID没有被注册
							{
								gOperateBit =0;
								BIT_MORE_TWO_WARM();
								lock_operate.id = pFun(0);
								SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							}
						}
						else
						{   //错误状态
							gOperateBit =0;
							BIT_MORE_TWO_WARM();
							lock_operate.id = pFun(0);
							SegDisplayCode = GetDisplayCodeNum(lock_operate.id);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );
							printf("err system\r\n");
						}
					}
				}
				else if(Delete_Mode_Temp == DELETE_USER_BY_FP)
				{
					if(e.data.KeyValude=='#')
					{
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_USER_BY_FP());  
					}
				}
				else if(Delete_Mode_Temp == DELETE_USER_ALL)
				{
					if(e.data.KeyValude=='#')
						Action_Delete_All_User_ID();
				}
				else if(Delete_Mode_Temp == DELETE_ADMIN_BY_FP)
				{
					if(e.data.KeyValude=='#')
					{
						SegDisplayCode = Lock_Enter_DELETE_ADMIN_BY_FP();
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );
					}
				}
				else if(Delete_Mode_Temp == DELETE_ADMIN_ALL)
				{
					if(e.data.KeyValude=='#')
						Action_Delete_All_Admin_ID();
				}
				else
				{//Err
					ERR_UNKNOWN(); 
					Delete_Mode_Temp = DELETE_USER_BY_FP;
					SegDisplayCode = GetDisplayCodeFP();
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode ); 
				}
			   if(e.data.KeyValude=='*')
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
							case KEY_CANCEL_LONG:
								SegDisplayCode = Lock_EnterReady();
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
								break;
							case KEY_DEL_SHORT:
							case KEY_DEL_LONG:
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
							case KEY_ADD_LONG:
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
							case KEY_OK_LONG:
								if((lock_operate.id>0)&&(lock_operate.id<=USER_ID_MAX))
								{
									if(Find_Next_User_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
									{
										Lock_Enter_Passwd_One();
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
					fifo_clear(&touch_key_fifo);
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
						
						if((lock_operate.id>0)&&(lock_operate.id<=USER_ID_MAX))
						{
							if(Find_Next_User_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
							{
								Lock_Enter_Passwd_One();
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
										Lock_Enter_Passwd_One();
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
					fifo_clear(&touch_key_fifo);
					if((e.data.KeyValude>='0')&&(e.data.KeyValude<='9'))
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
						if((lock_operate.id>USER_ID_MAX)&&(lock_operate.id<=ADMIN_ID_MAX))
						{
							if(Find_Next_Admin_Null_ID_Add(lock_operate.id-1)==lock_operate.id)
							{
								Lock_Enter_Passwd_One();
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
							Exit_Finger_Current_Operate();
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
						if(Compare_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1,0x03)==0)
						{
							ClearAllEvent();
							strcpy(gEventOne.data.Buff, touch_key_buf);
							Beep_PSWD_ONE_OK_Warm();
							gEventOne.len = TOUCH_KEY_PSWD_LEN;
							gEventOne.event = TOUCH_KEY_EVENT;
							lock_operate.lock_state = WATI_PASSWORD_TWO;
							Exit_Finger_Current_Operate();
						}
						else
							Beep_Register_Fail_Warm();
						fifo_clear(&touch_key_fifo);
					}
					else
					{
						if(e.data.KeyValude=='#')
						{	
							if(len == 1)
								fifo_clear(&touch_key_fifo);
							else if(len>TOUCH_KEY_PSWD_MIN_LEN)
							{
								len = len -1;
								touch_key_buf[len] = '\0';
								if((Compare_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1,0x03)==0)&&(CompareReverse_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1)==0))
								{
									Beep_PSWD_ONE_OK_Warm();
									gEventOne.len = len;
									gEventOne.event = TOUCH_KEY_EVENT;
									strcpy(gEventOne.data.Buff, touch_key_buf);
									lock_operate.lock_state = WATI_PASSWORD_TWO;
									Exit_Finger_Current_Operate();
								}
								else
								{
									Beep_Register_Fail_Warm();/////////////
								}
							}
							else
							{
								Beep_Register_Fail_Warm();
							}
							fifo_clear(&touch_key_fifo);
						}
						else if(e.data.KeyValude=='*')
						{
							if(len>1)
							{
								fifo_clear(&touch_key_fifo);
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
								Exit_Finger_Current_Operate();
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
					if(Compare_To_Flash_id(RFID_PSWD, RFID_CARD_NUM_LEN, (char*)e.data.Buff,1,0x03)==0)
					{
						ONE_WARM_BEEP();//Beep_PSWD_ONE_OK_Warm();
						gEventOne.event = RFID_CARD_EVENT;
						strcpy(gEventOne.data.Buff, e.data.Buff);
						lock_operate.lock_state = WATI_PASSWORD_TWO;
						Exit_Finger_Current_Operate();
					}
					else
						Beep_Register_Fail_Warm();    
					fifo_clear(&touch_key_fifo);
				}
				else if(e.event==FINGER_EVENT)
				{
#if 0
					if(e.data.Buff[0]==REGIST1_CMD)
					{
						//if(e.data.Buff[1]==ACK_SUCCESS)//第一指纹通过
						{
							Finger_Regist_CMD1(); 
							ONE_WARM_BEEP();//Beep_PSWD_ONE_OK_Warm();  //第一次指纹采样成功
							gEventOne.event = FINGER_EVENT;
							lock_operate.lock_state = WATI_PASSWORD_TWO;
						}
							
					}
					else
#endif
						if(e.data.Buff[0]==REGIST2_CMD)
					{
						
						if((e.data.Buff[1]==ACK_FAIL) || (e.data.Buff[1]==ACK_IMAGEFAIL) || (e.data.Buff[1]==ACK_USER_EXIST))
						{
							Finger_Regist_CMD2();  
							Beep_Register_Fail_Warm(); 
						}
						else if(e.data.Buff[1]==ACK_FULL)
						{
							Lock_FU_Indication();
						}
						else if(e.data.Buff[1]==ACK_SUCCESS)  
						{
							Key_Touch_Beep_Warm_block();//Beep_PSWD_ONE_OK_Warm();  //第一次指纹采样成功
//							delay_ms(300);
							Finger_Regist_CMD3(); 
							gEventOne.event = FINGER_EVENT;
							lock_operate.lock_state = WATI_PASSWORD_TWO;
							
						}
						else 
							Finger_Regist_CMD2();  //继续开启注册命令
					}
					else if(e.data.Buff[0]==REGIST3_CMD)
					{
						if((e.data.Buff[1]==ACK_FAIL) || (e.data.Buff[1]==ACK_IMAGEFAIL))
						{
							 
							Beep_Register_Fail_Warm(); 
							Finger_Regist_CMD2(); 
						}
						else if(e.data.Buff[1]==ACK_FULL)
						{
							Lock_FU_Indication();
						}
						else if(e.data.Buff[1]==ACK_SUCCESS)  //这个状态是错误的，，只有在passwd_two 状态下才能收到这个命令
						{
							id_infor_t id_infor;
							
							id_infor.id = lock_operate.id;
							id_infor.type = FINGER_PSWD;
							id_infor.len = FINGER_INDEX_LEN;
							id_infor.password[2] = e.data.Buff[1];
							id_infor.password[3] = e.data.Buff[2];
							PasswdTwoCompara_Sucess(id_infor);
						}
						else 
							Finger_Regist_CMD2();  //继续开启注册命令
					}
					else
					{
						
					}
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
					
					if(gEventOne.event == TOUCH_KEY_EVENT)
					{
						if((len==TOUCH_KEY_PSWD_LEN)&&(!((e.data.KeyValude=='#')||(e.data.KeyValude=='*'))))
						{
							touch_key_buf[len] = '\0';
							if((gEventOne.event==TOUCH_KEY_EVENT)&&(len==gEventOne.len)&&(strcmp(touch_key_buf, gEventOne.data.Buff)==0))
							{
								id_infor.id = lock_operate.id;
								id_infor.type = TOUCH_PSWD;
								id_infor.len = TOUCH_KEY_PSWD_LEN;
								strcpy(id_infor.password, touch_key_buf);	
								
								PasswdTwoCompara_Sucess(id_infor);	
							}
							else
							{
								
								PASSWD_COMPARE_ERR();
								Lock_Enter_Passwd_One();
								//toddo
							}
							memset(&gEventOne, 0, sizeof(EventDataTypeDef));
							fifo_clear(&touch_key_fifo);
						}
						else if(e.data.KeyValude=='#')
						{
							len = len -1;
							touch_key_buf[len] = '\0';
							if(len>=TOUCH_KEY_PSWD_MIN_LEN)
							{
								if(((gEventOne.event==TOUCH_KEY_EVENT)&&(gEventOne.len == len)&&(strcmp(touch_key_buf, gEventOne.data.Buff)==0)))
								{
									id_infor.id = lock_operate.id;
									id_infor.type = TOUCH_PSWD;
									id_infor.len = len;
									strcpy(id_infor.password, touch_key_buf);	
									
									PasswdTwoCompara_Sucess(id_infor);								
								}
								else
								{
									Lock_Enter_Passwd_One();
									PASSWD_COMPARE_ERR();
									//toddo
								}
							}
							fifo_clear(&touch_key_fifo);
						}
					}
					else
						fifo_clear(&touch_key_fifo);
				    if(e.data.KeyValude=='*')
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
						
						PasswdTwoCompara_Sucess(id_infor);
					}
					else
					{
						Lock_Enter_Passwd_One();
						PASSWD_COMPARE_ERR();
						//toddo
					}
					memset(&gEventOne, 0, sizeof(EventDataTypeDef));
					fifo_clear(&touch_key_fifo);
				}
				else if(e.event==FINGER_EVENT)
				{				
					if(gEventOne.event == FINGER_EVENT)
					{
						if(e.data.Buff[0]==REGIST1_CMD)
						{	
							Finger_Regist_CMD1(); 
							ONE_WARM_BEEP();//Beep_PSWD_ONE_OK_Warm();  //第2,3 ...次指纹采样成功
						}
						else if(e.data.Buff[0]==REGIST3_CMD)
						{
							if(e.data.Buff[1]==ACK_SUCCESS)
							{
								uint16_t id;
								
								id = e.data.Buff[2] + e.data.Buff[3]*256;
								if(id>0)//注册成功
								{
									id_infor_t id_infor;
									
									id_infor.id = lock_operate.id;
									id_infor.type = FINGER_PSWD;
									id_infor.len = FINGER_INDEX_LEN;
									id_infor.password[0] = e.data.Buff[2];
									id_infor.password[1] = e.data.Buff[3];
									
									PasswdTwoCompara_Sucess(id_infor);
								}	
								else//注册失败
								{
									PASSWD_COMPARE_ERR();
									Lock_Enter_Passwd_One();
									
								}
							}
							else if(e.data.Buff[1]==ACK_FULL)
							{
								Lock_FU_Indication();
							}
							else//((e.data.Buff[0]==ACK_FAIL) || (e.data.Buff[0]==ACK_IMAGEFAIL))
							{
								PASSWD_COMPARE_ERR();
//								delay_ms(300);
								printf("delay 500ms \r\n");
								Lock_Enter_Passwd_One();
								
							}
						}
					}
					UsartClrBuf();
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
						}
						else
						{
							SegDisplayCode = Lock_EnterReady();
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//
						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))||((e.data.KeyValude=='#')&&(len>TOUCH_KEY_PSWD_MIN_LEN)))
					{	
						if(e.data.KeyValude=='#')
							len = len -1;
						touch_key_buf[len] = '\0';
						if(0 !=Compare_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1,0x02))
						{
							WaitAuthentic_OK();	
							Exit_Finger_Current_Operate();
						}					
						else
						{
//							PASSWD_COMPARE_ERR();
//							fifo_clear(&touch_key_fifo);
							ReadyState_CompareErrCount_Add(3);
						}
					}
					else if((e.data.KeyValude=='#')&&(len<=TOUCH_KEY_PSWD_MIN_LEN))  //长度包括#
					{
						ReadyState_CompareErrCount_Add(3);
//						PASSWD_COMPARE_ERR();
//						fifo_clear(&touch_key_fifo);
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					if(0 !=Compare_To_Flash_id(RFID_PSWD,RFID_CARD_NUM_LEN, (char*)e.data.Buff,1,0x02))
					{
						WaitAuthentic_OK();	
						Exit_Finger_Current_Operate();						
					}	
					else
						PASSWD_COMPARE_ERR(); 
				}
				else if(e.event==FINGER_EVENT)
				{
					if(e.data.Buff[0]==MATCH_CMD)
					{
						uint16_t finger_num;
						finger_num = e.data.Buff[2] + e.data.Buff[3]*256;
						if(finger_num!=0)
						{
							int8_t id;
							
							id = Get_Finger_Admin_From_InterIndex(finger_num);
							if(id!=0)
								WaitAuthentic_OK();	
							else //错误状态，指纹头里注册过，但是在flash index中找不到
							{
								Delelte_ONE_Finger(finger_num);
								Lock_EnterIdle();
							}
						}
						else //错误
						{
							
							PASSWD_COMPARE_ERR(); 
//							delay_ms(300);
							Lock_Enter_Authntic();
						}
							
					}
					UsartClrBuf();
				}
				break;
			case DELETE_USER_BY_FP:
				if(e.event==BUTTON_KEY_EVENT)
				{

					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							
							Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
							Exit_Finger_Current_Operate();	
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
							Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
							Exit_Finger_Current_Operate();	

						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN))||((e.data.KeyValude=='#')&&(len>=TOUCH_KEY_PSWD_MIN_LEN)))
					{	
						if(e.data.KeyValude=='#')
							len = len -1;
						touch_key_buf[len] = '\0';

						id = Compare_To_Flash_id(TOUCH_PSWD, len, (char*)touch_key_buf,1,0x03);
						if(id !=0)//
						{
							if((id>0)&&(id<=USER_ID_MAX))
							{
								Delect_One_ID((uint8_t) id);
								PASSWD_COMPARE_OK();
								Delete_ID_Flash_Once(id);
								if(Get_User_id_Number()==0)
								{
									Lock_NULL_Indication();
								}
								Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
								Exit_Finger_Current_Operate();	
							}
							else
							{
								PASSWD_COMPARE_ERR();
								fifo_clear(&touch_key_fifo);
							}

						}		
						else
						{
							PASSWD_COMPARE_ERR();
							fifo_clear(&touch_key_fifo);
//							Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
//							Exit_Finger_Current_Operate();	
						}
					}
					else if(((e.data.KeyValude=='#')&&(len<=TOUCH_KEY_PSWD_MIN_LEN)))
					{
						PASSWD_COMPARE_ERR();
						fifo_clear(&touch_key_fifo);
//						Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
//						Exit_Finger_Current_Operate();	
					}
					
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					id = Compare_To_Flash_id(RFID_PSWD, RFID_CARD_NUM_LEN, (char*)e.data.Buff,0,0x03);
					if(id!=0)
					{
						if((id>0)&&(id<=USER_ID_MAX))
						{
							Delect_One_ID((uint8_t) id);
							PASSWD_COMPARE_OK();
							Delete_ID_Flash_Once(id);
							if(Get_User_id_Number()==0)
							{
								Lock_NULL_Indication();
							}
							Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
							Exit_Finger_Current_Operate();	
						}
						else
						{
							PASSWD_COMPARE_ERR();
						}

					}
					else 
						PASSWD_COMPARE_ERR(); 
					fifo_clear(&touch_key_fifo);
				}
				else if(e.event==FINGER_EVENT)
				{
					if(e.data.Buff[0]==MATCH_CMD)
					{
						uint16_t finger_num;
						
						finger_num = e.data.Buff[3]*256 + e.data.Buff[2];
						
						if(finger_num!=0)//比对成功
						{						
							int8_t id;
							
							id = Get_Finger_User_From_InterIndex(finger_num);
							if((id>0)&&(id<=USER_ID_MAX))
							{
								Delect_One_ID((uint8_t) id);
								PASSWD_COMPARE_OK();
								Delete_ID_Flash_Once(id);
								if(Get_User_id_Number()==0)
								{
									Lock_NULL_Indication();
								}
								Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
								Exit_Finger_Current_Operate();
							}
							else
							{
									PASSWD_COMPARE_ERR();
									Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_USER_BY_FP() ); 
							}
	
						}
						else 
						{	
							PASSWD_COMPARE_ERR();
//							delay_ms(300);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_USER_BY_FP() ); 
	
						}
							
//						Wait_Select_Delete_Mode(DELETE_USER_BY_FP);
					}
					
				}
					
				break;
			case DELETE_ADMIN_BY_FP:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case	KEY_CANCEL_SHORT:
							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
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
							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
							Exit_Finger_Current_Operate();
							
						}
							
					}
					else if(((len>=TOUCH_KEY_PSWD_LEN)&&(len<=TOUCH_KEY_PSWD_MAX_LEN)) || ((e.data.KeyValude=='#')&&(len>TOUCH_KEY_PSWD_MIN_LEN)))
					{
						if(e.data.KeyValude=='#')
							len = len - 1;
						touch_key_buf[len] = '\0';
						id = Compare_To_Flash_id(TOUCH_PSWD, len,(char*)touch_key_buf,1,0x02);
						if(id !=0)
						{
							if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
							{
								PASSWD_COMPARE_OK();//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								Delect_One_ID((uint8_t) id);
								Delete_ID_Flash_Once(id);
								if(Get_Admin_id_Number()==0)
								{
									Set_Work_Mode(NORMAL);
									Lock_NULL_Indication();
								}
							}
							else
								PASSWD_COMPARE_ERR();
							
							fifo_clear(&touch_key_fifo);
							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
							Exit_Finger_Current_Operate();
						}	
						else
						{
							PASSWD_COMPARE_ERR();  
							fifo_clear(&touch_key_fifo);
//							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
						}						
					}
					else if((e.data.KeyValude=='#')&&(len<=TOUCH_KEY_PSWD_MIN_LEN))
					{
						PASSWD_COMPARE_ERR(); 
//						Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
						fifo_clear(&touch_key_fifo);
					}
				}
				else if(e.event==RFID_CARD_EVENT)
				{
					    id = Compare_To_Flash_id(RFID_PSWD, RFID_CARD_NUM_LEN, (char*)e.data.Buff,1,0x02);
						if(id !=0)
						{
							if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
							{
								PASSWD_COMPARE_OK();
								Delect_One_ID((uint8_t) id);
								Delete_ID_Flash_Once(id);
								if(Get_Admin_id_Number()==0)
								{
									Set_Work_Mode(NORMAL);
									Lock_NULL_Indication();
								}
							}
							else
								PASSWD_COMPARE_ERR();
							
							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
						}
						else
						{
							PASSWD_COMPARE_ERR(); 
//							Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
							fifo_clear(&touch_key_fifo);
						}
							
				}
				else if(e.event==FINGER_EVENT)
				{
					if(e.data.Buff[0]==MATCH_CMD)
					{
						uint16_t finger_num;
						
						finger_num = e.data.Buff[3]*256 + e.data.Buff[2];
						
						if(finger_num!=0)//比对成功
						{						
							int8_t id;
							
							id = Get_Finger_Admin_From_InterIndex(finger_num);
							if((id>USER_ID_MAX)&&(id<=ADMIN_ID_MAX))
							{
								Delect_One_ID((uint8_t) id);
								PASSWD_COMPARE_OK();
								Delete_ID_Flash_Once(id);
								if(Get_Admin_id_Number()==0)
								{
									Set_Work_Mode(NORMAL);
									Lock_NULL_Indication();
								}
								Wait_Select_Delete_Mode(DELETE_ADMIN_BY_FP);
								Exit_Finger_Current_Operate();
							}
							else
							{
								PASSWD_COMPARE_ERR();
	//							delay_ms(300);
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_ADMIN_BY_FP() ); 
							}
								
						}
						else 
						{	
							PASSWD_COMPARE_ERR();
//							delay_ms(300);
							Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_Enter_DELETE_ADMIN_BY_FP() ); 
						}

					}
					
				}
					
				break;
			case ADD_ID_OK:
			case DELETE_ID_OK:
				break;
			case LOCK_OPEN:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Motor_Drive_Reverse();
							delay_ms(800);
							Lock_EnterIdle();
							break;
						default:
							break;
					}
				}
				if(motor_state==MOTOR_NONE)
				{
					motor_state = MOTOR_FORWARDK;
					MotorEndTime = GetSystemTime() + 200;
					Motor_Drive_Forward();
				}
				else
				{
					if(GetSystemTime() > MotorEndTime)
					{
						motor_state = MOTOR_NONE;
//						lock_operate.pDooInfor->door_state = 1;
						Motor_Drive_Stop();
//						SegDisplayCode = Lock_EnterReady();
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_EnterReady() );//显示--或者u n
					}
				}
	
				break;
			case LOCK_CLOSE:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Motor_Drive_Reverse();
							delay_ms(200);
							Lock_EnterIdle();
							break;
						default:
							break;
					}
				}
				if(motor_state==MOTOR_NONE)
				{
					motor_state = MOTOR_REVERSE;
					MotorEndTime = GetSystemTime() + 200;
//					printf("lock close time= %d\r\n",GetSystemTime());
					Motor_Drive_Reverse();
					
				}
				else
				{
					if(GetSystemTime() > MotorEndTime)
					{
//						printf("lock stop time= %d\r\n",GetSystemTime());
						motor_state = MOTOR_NONE;
						Motor_Drive_Stop();
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, Lock_EnterReady() );
					}
				}
				break;
			case LOCK_OPEN_CLOSE:
				if(e.event==BUTTON_KEY_EVENT)
				{
					switch(e.data.KeyValude)
					{
						case KEY_CANCEL_SHORT:
						case KEY_CANCEL_LONG:
							Motor_Drive_Reverse();
							delay_ms(200);
							Lock_EnterIdle();
							break;
						default:
							break;
					}
				}
				else if(e.event == TOUCH_KEY_EVENT)
				{
					fifo_clear(&touch_key_fifo);
					if((e.data.KeyValude==(LONG_KEY_MASK|'#'))&&(motor_state==MOTOR_STOP))
					{
						uint16_t vol;
						
						vol = Get_Battery_Vol();
						if(vol<=4800)
						{
								Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, GetDisplayCodeBatteryLowlMode() );
								Battery_Low_Warm();
								MotorEndTime = GetSystemTime();
						}
						else
						{
							lock_operate.lock_state = LOCK_OPEN_NORMAL;
							Write_Open_Normally_Flag();
							LOCK_ENTER_NOMAL_MODE_WARM();
							Lock_EnterIdle();
						}
					}
				}
				if(motor_state==MOTOR_NONE)
				{
					printf("moto forward\r\n");
					motor_state = MOTOR_FORWARDK;
					MotorEndTime = GetSystemTime() + 250;//t200 200
					Motor_Drive_Forward();
				}
				else if(motor_state==MOTOR_FORWARDK)
				{
					if(GetSystemTime() >= MotorEndTime)
					{
						printf("moto stop\r\n");
						motor_state = MOTOR_STOP;
						MotorEndTime = GetSystemTime() + 3000;
						Motor_Drive_Stop();
					}
				}
				else if(motor_state==MOTOR_STOP)
				{
					if(GetSystemTime() >= MotorEndTime)
					{
						printf("moto reverse\r\n");
						motor_state = MOTOR_REVERSE;
						MotorEndTime = GetSystemTime() + 250;//t200 200
						Motor_Drive_Reverse();
					}
				}
				else 
				{
					if(GetSystemTime() > MotorEndTime)
					{
						printf("moto stop, enter idle\r\n");
						//motor_state = MOTOR_NONE;
//						lock_operate.pDooInfor->door_state = 1;
						Motor_Drive_Stop();
						Lock_EnterIdle();
						//Hal_SEG_LED_Display_Set(HAL_LED_MODE_FLASH, SegDisplayCode );//显示--或者u n
					}
				}
				break;
			case LOCK_OPEN_NORMAL:
				if((e.event==TOUCH_KEY_EVENT) && (e.data.KeyValude==(LONG_KEY_MASK|'#')))
				{
					//fifo_clear(&touch_key_fifo);
					MotorEndTime = GetSystemTime()+150;
					motor_state = MOTOR_REVERSE;
					Motor_Drive_Reverse();
					LOCK_ENTER_NOMAL_MODE_WARM();
					printf("endtime = %d\r\n", MotorEndTime);
//					delay_ms(200);
//					Motor_Drive_Stop();
//					Erase_Open_Normally_Mode();
//					LOCK_ENTER_NOMAL_MODE_WARM();
//					Lock_EnterIdle();
				}
				else if(e.event!=EVENT_NONE)
				{
					if(e.event==TOUCH_KEY_EVENT)
						fifo_clear(&touch_key_fifo);
					SleepTime_End = GetSystemTime() + 3000;
//					LOCK_OPEN_NORMAL_MODE_Warm();
				//	Lock_EnterIdle();
				}
				else if(motor_state == MOTOR_REVERSE)
				{
					printf("mot\r\n");
					if(GetSystemTime() >= MotorEndTime)
					{
						printf("systime = %d\r\n",GetSystemTime());
						Motor_Drive_Stop();
						Erase_Open_Normally_Mode();
						Lock_EnterIdle();	
					}
				}
				break;
			case LOCK_GET_ID_NUM:
				if(e.event==BUTTON_KEY_EVENT)
				{
					if(e.data.KeyValude == KEY_CANCEL_SHORT)
					{
						SegDisplayCode = Lock_EnterReady();
						Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//	
					}
				}
				else if(e.event==TOUCH_KEY_EVENT)
				if(e.data.KeyValude == '#')
				{
					SegDisplayCode = Lock_EnterReady();
					Hal_SEG_LED_Display_Set(HAL_LED_MODE_ON, SegDisplayCode );//	
				}
				break;
			default :
				break;
	}
}




