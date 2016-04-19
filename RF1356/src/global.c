/*
* Copyright (c) 2012,上海图安信息技术有限公司
* All rights reserved.
*
* 工程名称：1211-C指纹锁项目
* 文件名称：global.c
* 文件标识：无
* 摘    要：全局变量定义源文件
*
* 当前版本：2.0
* 作    者：汪华红
* 邮    箱：wanghuahong@tooan.cn
* 起始日期：2012年09月30日
* 完成日期：
*
* 编译环境：Keil V4.23.00.0
* 芯片型号：STM32F205VG
*/

#include "RF1356.h"


//finger.c

//指纹应用同操作EEPROM一样，分成独立的小应用，通过g_cFingerOperatingStatus传递当前的具体应用
//具体实现在低优先级循环里执行。g_cFingerOperatingDataBuff[0~2]将传递结果及数据，
//g_cFingerOperatingDataBuff[0] 表示当前应用的成功与失败或者其它。后续BUFF内容表示传入数据内容。
//g_cFingerOperatingDataBuff[1]
//g_cFingerOperatingDataBuff[2]
//0 1 2 从底层返回
//g_cFingerOperatingDataBuff[3];//用户编号
//g_cFingerOperatingDataBuff[4]	//用户编号
//g_cFingerOperatingDataBuff[5];//用户权限
unsigned char g_cFingerOperatingStatus=0xFF;
unsigned char g_cFingerOperatingDataBuff[10];
int g_cFingerUsetAmount = 0;
unsigned char g_cFingerFeaBuffA[512];
unsigned char g_cFingerFeaBuffB[512];
unsigned char g_cFingerFeaBuffC[512];
unsigned char g_cFingerFeaBuffD[512];   // added by sky for enrollment
unsigned char g_cFingerAllowRepeatReg;
unsigned char g_cGetImageFlag;
unsigned char g_cFingerAdminCheckBuff[2];

unsigned char g_cTiaoShiBuff[30] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};
//key.c
unsigned char g_ckeyValue[15] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};    //0,nokeyValue  1,have keyvolue  
unsigned char g_ckeyState[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //0 1 2 3 4 5 6 7 8 9 * # S(复位键) 长按“*”  长按“#” 
unsigned char g_ckeyTime[15]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char g_cKeyCurrentValue=0xFF;//0:0  1:1  2:2  3:3  4:4  5:5  6:6  7:7  8:8  9:9  10:*  11:#  12:复位键  13:长按“*” 14:长按“#”
unsigned char g_cKeyTouchCount=0;
unsigned char g_cKeyTouchBuff[4];
unsigned short g_cKeyResetTime;
unsigned char g_cTouchKeyStart = 0;
unsigned char g_cKeyUpKeyValue = 0xFF;


//系统工作参数
unsigned char g_cDeviceUseOfficeFlag=0;
unsigned char g_cDeviceLockFlag=0;		//设备锁机标识	 电子部分失效
//unsigned char g_cDeviceStatus=0x01;//当前设备工作状态,复位后使它处于刚上电状态

unsigned char g_cDevicePowerOnTestFlag=0;//设备上电直接进入测试阶段标识，该位置1，进入老化测试流程。
unsigned short g_sDeviceTestCount=0;
unsigned char g_cDeviceAlwaysOpenFlag=0;//门常开标志
unsigned char g_cTempErrorTime;//临时错误次数
unsigned char g_cOpenDoorCheckBuff[6];	   //每次验证开门的ID号以及类型
unsigned char g_cSafeModeCheckOptions = 0;//正确输入密码的次数
unsigned char g_cRTC_Wakeup_Flag = 0;
volatile unsigned char g_c200MS_GetCard = 0;
unsigned char g_c200MS_ReadKeyFlag = 0;
unsigned char g_c200MS_ReadKeyTimeTip = 0;	   //读触摸键盘时间片
unsigned char g_cMeasurePlusOver = 0;
unsigned short g_cUsrnum = 0;
unsigned char g_cCardnum = 0;



unsigned char g_cDeviceCheckMode=0;//0：单认证模式；1：双重认证；2：三重认证
unsigned char g_cOpenDoorErrorCount=0xFF;//开门错误计数计数器
unsigned char g_cUserAmount=0;//用户总数
unsigned char g_cAdminAmount=0;//管理员总数
unsigned char g_cPerUserAmount=0;//常驻用户总数
unsigned char g_cTempUserAmount=0;//临时用户总数




//EEPROM读写状态
unsigned char g_cEepromWrStatus = 0xFF;
unsigned char g_cEepromRdStatus = 0xFF;

//EEPROM读写缓冲
unsigned char g_cEepromWrBuff[20];
unsigned char g_cEepromRdBuff[55];


//beep.c
unsigned char g_cBeepDiNum=0;
unsigned short g_cBeepDiAllTime=40;
unsigned char g_cBeepDiTime=0;
unsigned char g_cBeepDiLongFlag=0;
unsigned char g_cBeepMod = 0;

//light.c
unsigned char g_cRedFlashNum=0;
unsigned char g_cRedFlashTime=0;
unsigned short g_cRedAllTime = 40;
unsigned char g_cGreenAllTime = 40;
unsigned char g_cRedLightFlag=0;

unsigned char g_cGreenFlashNum=0;
unsigned char g_cGreenFlashTime=0;
unsigned char g_cGreenLightFlag=0;

unsigned char g_cBackLightFlag=0;

//报警控制
unsigned char g_cAlarmFlag=0;
unsigned int g_cAlarmCount=0;
unsigned short g_cAlarmTimerCount;

//ADC电压检测
unsigned short g_sAdcValue;
unsigned short g_sAdcValue1;
unsigned char g_cLowPwoerCount;
unsigned char g_cVeryLowPwoerCount;
unsigned char g_cLowPowerAlarmFlag;

//RTC
unsigned short g_cRtcLastSecValue;
unsigned char g_cRtcEnableUpdateFlag;
unsigned char g_cDate[10] ={0xB8,0xB6,0xB6,0xB6,0xC3,0xB6,0xB6,0xC3,0xB6,0xB6}; // 2000-00-00
unsigned char g_cTime[8] = {0xB6,0xB6,0xC2,0xB6,0xB6,0xC2,0xB6,0xB6};// 00:00:00
unsigned char g_cLcdXiuGaiRQ[10];
unsigned char g_cLcdXiuGaiSJ[10];
//unsigned char g_cDate1[10] ={0xB8,0xB6,0xB6,0xB6,0xC3,0xB6,0xB6,0xC3,0xB6,0xB6}; // 2000-00-00
//unsigned char g_cTime1[8] = {0xB6,0xB6,0xC2,0xB6,0xB6,0xC2,0xB6,0xB6};// 00:00:00

//sleep.c
unsigned char g_cSleepFlag;
unsigned char g_cSleep_Mod;


//motor.c
unsigned char g_cMotorDriveStatus=0xFF;//电机转动状态，分正反转，和正转
unsigned int g_sMotorDriveCount;//用于开关门的时候进行计时
unsigned char g_cMotorDriveStep;
unsigned char g_cOpenDoor_Mod = 0xFF;	//0xF0 常开模式    0xFF  正常模式	  0x00  空闲模式




//card.c
#define RFID1356_READ_CARD_OK                       0x02
#define RFID1356_DISABLE_CARD_RX                    0x01
#define RFID1356_ENABLE_CARD_RX                     0x00
volatile unsigned char g_cGetCardStatus = RFID1356_ENABLE_CARD_RX;
unsigned char g_cCardUserAmount = 0;
unsigned char g_cRfidRxBuff[20];
unsigned char g_cInitStatusEnRdCardFlag;

//密码开门使用缓冲区
unsigned char g_cPasswordBuff[16];
unsigned char g_cPasswordCount = 0;
unsigned char g_cPasswordCount1 = 0;
unsigned char g_cAddPasswordbuff[2][16];
	   	


//password.c
unsigned char g_cPasswordOperatingStatus=0xFF;
unsigned char g_cPasswordOperatingDataBuff[2][15];

//voice.c
unsigned char g_cVoiceOpenFlag;
unsigned char g_cVoiceBroadcastData=0xFF;




//flash.c
//FLASH读写状态
unsigned char g_cFLASHWrStatus = 0xFF; 
unsigned char g_cFLASHRdStatus = 0xFF;

//FLASH读写缓冲
unsigned char g_cWrFLASHbuff[20];
unsigned char g_cRdFLASHbuff[55];

unsigned char readflash = 0;
unsigned char writeflash = 0;

//pwm.c
unsigned short g_cCapture = 0;
unsigned short g_cCapture2 = 0;
unsigned char g_cCaptureNumber = 0;
unsigned short g_cIC3ReadValue1 = 0;
unsigned short g_cIC3ReadValue2 = 0;


//擦除区  数据缓冲
unsigned char g_cEraseSectionBuff[1024];
//unsigned char g_cEraseSectionBuff[16];

