/*
* Copyright (c) 2012,上海图安信息技术有限公司
* All rights reserved.
*
* 工程名称：1211-C指纹锁项目
* 文件名称：global.h
* 文件标识：无
* 摘    要：全局变量定义头文件
*
* 当前版本：2.0
* 作    者：汪华红
* 邮    箱：wanghuahong@tooan.cn
* 起始日期：2012年09月30日
* 完成日期：
*
* 编译环境：Keil V4.23.00.0
* 芯片型号：STM32F207VG
*/


#ifndef _GLOBAL_H
#define _GLOBAL_H



extern unsigned char g_cTiaoShiBuff[30];


//key.c
extern unsigned char g_ckeyValue[15];
extern unsigned char g_ckeyState[15];
extern unsigned char g_ckeyTime[15];
extern unsigned char g_cKeyCurrentValue;
extern unsigned char g_cKeyTouchCount;
extern unsigned char g_cKeyTouchBuff[4];
extern unsigned short g_cKeyResetTime;

extern unsigned char g_cTouchKeyStart;
extern unsigned char g_cKeyUpKeyValue;
extern volatile unsigned char g_cReadkeyFlag;



//系统工作参数
extern unsigned char g_cDeviceUseOfficeFlag;
extern unsigned char g_cDeviceLockFlag;	//电子部分失效
extern unsigned char g_cDeviceStatus;//当前设备工作状态	 
extern unsigned char g_cDevicePowerOnTestFlag;//设备上电直接进入测试阶段标识，该位置1，进入老化测试流程。
extern unsigned short g_sDeviceTestCount;
extern unsigned char g_cDeviceAlwaysOpenFlag;//门常开标志
extern unsigned char g_cTempErrorTime;//临时错误次数
extern unsigned char g_cSafeModeCheckOptions;
extern unsigned char g_cOpenDoorCheckBuff[6];	   //每次验证开门的ID号以及类型
extern unsigned char g_cRTC_Wakeup_Flag;
extern volatile unsigned char g_c200MS_GetCard; 
extern unsigned char g_c200MS_ReadKeyFlag;
extern unsigned char g_c200MS_ReadKeyTimeTip;	   //读触摸键盘时间片
extern unsigned char g_cMeasurePlusOver;
extern unsigned short g_cUsrnum;
extern unsigned char g_cCardnum;




extern unsigned char g_cDeviceCheckMode;//0：单认证模式；1：双重认证；2：三重认证
extern unsigned char g_cOpenDoorErrorCount;//开门错误计数计数器
extern unsigned char g_cUserAmount;//用户总数
extern unsigned char g_cAdminAmount;//管理员总数
extern unsigned char g_cPerUserAmount;//常驻用户总数
extern unsigned char g_cTempUserAmount;//临时用户总数

//Menu.c
extern volatile unsigned char g_cMenuNum;
extern volatile unsigned char g_cMenuStatus;
extern volatile unsigned int  g_sMenuDelayTime;
extern unsigned char g_cMenuSubNum;
extern unsigned char g_cMenuSubStatus;
extern unsigned char g_cMenuSubBuff[10];
extern unsigned char g_cMenuBuff[10];

//EEPROM读写状态
extern unsigned char g_cEepromWrStatus;
extern unsigned char g_cEepromRdStatus;

//EEPROM读写缓冲
extern unsigned char g_cEepromWrBuff[20];
extern unsigned char g_cEepromRdBuff[55];

//beep.c
extern unsigned char g_cBeepDiNum;
extern unsigned short g_cBeepDiAllTime;
extern unsigned char g_cBeepDiTime;
extern unsigned char g_cBeepDiLongFlag;
extern unsigned char g_cBeepMod;


//light.c
extern unsigned char g_cRedFlashNum;
extern unsigned char g_cRedFlashTime;
extern unsigned char g_cRedLightFlag;
extern unsigned short g_cRedAllTime;
extern unsigned char g_cGreenAllTime;

extern unsigned char g_cGreenFlashNum;
extern unsigned char g_cGreenFlashTime;
extern unsigned char g_cGreenLightFlag;

extern unsigned char g_cBackLightFlag;

//拆机报警控制
extern unsigned char g_cAlarmFlag;
extern unsigned int g_cAlarmCount;
extern unsigned short g_cAlarmTimerCount;

//motor.c
extern unsigned char g_cMotorDriveStatus;//电机转动状态，分正反转，和正转
extern unsigned int g_sMotorDriveCount;//用于开关门的时候进行计时
extern unsigned char g_cMotorDriveStep;
extern unsigned char g_cOpenDoor_Mod;	//0xF0 常开模式    0xFF  正常模式	  0x00  空闲模式

//ADC电压检测
extern unsigned short g_sAdcValue;
extern unsigned short g_sAdcValue1;
extern unsigned char g_cLowPwoerCount;
extern unsigned char g_cVeryLowPwoerCount;
extern unsigned char g_cLowPowerAlarmFlag;

//RTC
extern unsigned short g_cRtcLastSecValue;
extern unsigned char g_cRtcEnableUpdateFlag;
extern unsigned char g_cDate[10];
extern unsigned char g_cTime[8];
extern unsigned char g_cLcdXiuGaiRQ[10];
extern unsigned char g_cLcdXiuGaiSJ[10];
//extern unsigned char g_cDate1[10];
//extern unsigned char g_cTime1[8];

//sleep.c
extern unsigned char g_cSleepFlag;
extern unsigned char g_cSleep_Mod;




//card.c
extern volatile unsigned char g_cGetCardStatus;
extern unsigned char g_cCardUserAmount;
extern unsigned char g_cRfidRxBuff[20];
extern unsigned char g_cInitStatusEnRdCardFlag;


//密码开门使用缓冲区
extern unsigned char g_cPasswordBuff[16];
extern unsigned char g_cPasswordCount;
//密码注册使用缓冲区
extern unsigned char g_cAddPasswordbuff[2][16];
extern unsigned char g_cPasswordCount1;



//finger.c
extern unsigned char g_cFingerOperatingStatus;
extern unsigned char g_cFingerOperatingDataBuff[10];
extern int g_cFingerUsetAmount;
extern unsigned char g_cFingerFeaBuffA[];
extern unsigned char g_cFingerFeaBuffB[];
extern unsigned char g_cFingerFeaBuffC[];
extern unsigned char g_cFingerFeaBuffD[];
extern unsigned char g_cFingerAllowRepeatReg;
extern unsigned char g_cGetImageFlag;
extern unsigned char g_cFingerAdminCheckBuff[2];


//password.c
extern unsigned char g_cPasswordOperatingStatus;
extern unsigned char g_cPasswordOperatingDataBuff[2][15];

//字库
extern const unsigned char g_cCode[][16];
extern const unsigned char g_cAscii[][16];

//voice.c
extern unsigned char g_cVoiceOpenFlag;
extern unsigned char g_cVoiceBroadcastData;


//flash.c
//FLASH读写状态
extern unsigned char g_cFLASHWrStatus;
extern unsigned char g_cFLASHRdStatus;

//FLASH读写缓冲
extern unsigned char g_cWrFLASHbuff[20];
extern unsigned char g_cRdFLASHbuff[55];

extern unsigned char readflash;
extern unsigned char writeflash;


//pwm.c
extern unsigned short g_cCapture;
extern unsigned short g_cCapture2;

extern unsigned char g_cCaptureNumber;
extern unsigned short g_cIC3ReadValue1;
extern unsigned short g_cIC3ReadValue2;


//擦除区  数据缓冲
extern unsigned char g_cEraseSectionBuff[1024];
//extern unsigned char g_cEraseSectionBuff[16];



#endif
