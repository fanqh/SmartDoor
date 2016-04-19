/*
* Copyright (c) 2012,�Ϻ�ͼ����Ϣ�������޹�˾
* All rights reserved.
*
* �������ƣ�1211-Cָ������Ŀ
* �ļ����ƣ�global.h
* �ļ���ʶ����
* ժ    Ҫ��ȫ�ֱ�������ͷ�ļ�
*
* ��ǰ�汾��2.0
* ��    �ߣ�������
* ��    �䣺wanghuahong@tooan.cn
* ��ʼ���ڣ�2012��09��30��
* ������ڣ�
*
* ���뻷����Keil V4.23.00.0
* оƬ�ͺţ�STM32F207VG
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



//ϵͳ��������
extern unsigned char g_cDeviceUseOfficeFlag;
extern unsigned char g_cDeviceLockFlag;	//���Ӳ���ʧЧ
extern unsigned char g_cDeviceStatus;//��ǰ�豸����״̬	 
extern unsigned char g_cDevicePowerOnTestFlag;//�豸�ϵ�ֱ�ӽ�����Խ׶α�ʶ����λ��1�������ϻ��������̡�
extern unsigned short g_sDeviceTestCount;
extern unsigned char g_cDeviceAlwaysOpenFlag;//�ų�����־
extern unsigned char g_cTempErrorTime;//��ʱ�������
extern unsigned char g_cSafeModeCheckOptions;
extern unsigned char g_cOpenDoorCheckBuff[6];	   //ÿ����֤���ŵ�ID���Լ�����
extern unsigned char g_cRTC_Wakeup_Flag;
extern volatile unsigned char g_c200MS_GetCard; 
extern unsigned char g_c200MS_ReadKeyFlag;
extern unsigned char g_c200MS_ReadKeyTimeTip;	   //����������ʱ��Ƭ
extern unsigned char g_cMeasurePlusOver;
extern unsigned short g_cUsrnum;
extern unsigned char g_cCardnum;




extern unsigned char g_cDeviceCheckMode;//0������֤ģʽ��1��˫����֤��2��������֤
extern unsigned char g_cOpenDoorErrorCount;//���Ŵ������������
extern unsigned char g_cUserAmount;//�û�����
extern unsigned char g_cAdminAmount;//����Ա����
extern unsigned char g_cPerUserAmount;//��פ�û�����
extern unsigned char g_cTempUserAmount;//��ʱ�û�����

//Menu.c
extern volatile unsigned char g_cMenuNum;
extern volatile unsigned char g_cMenuStatus;
extern volatile unsigned int  g_sMenuDelayTime;
extern unsigned char g_cMenuSubNum;
extern unsigned char g_cMenuSubStatus;
extern unsigned char g_cMenuSubBuff[10];
extern unsigned char g_cMenuBuff[10];

//EEPROM��д״̬
extern unsigned char g_cEepromWrStatus;
extern unsigned char g_cEepromRdStatus;

//EEPROM��д����
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

//�����������
extern unsigned char g_cAlarmFlag;
extern unsigned int g_cAlarmCount;
extern unsigned short g_cAlarmTimerCount;

//motor.c
extern unsigned char g_cMotorDriveStatus;//���ת��״̬��������ת������ת
extern unsigned int g_sMotorDriveCount;//���ڿ����ŵ�ʱ����м�ʱ
extern unsigned char g_cMotorDriveStep;
extern unsigned char g_cOpenDoor_Mod;	//0xF0 ����ģʽ    0xFF  ����ģʽ	  0x00  ����ģʽ

//ADC��ѹ���
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


//���뿪��ʹ�û�����
extern unsigned char g_cPasswordBuff[16];
extern unsigned char g_cPasswordCount;
//����ע��ʹ�û�����
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

//�ֿ�
extern const unsigned char g_cCode[][16];
extern const unsigned char g_cAscii[][16];

//voice.c
extern unsigned char g_cVoiceOpenFlag;
extern unsigned char g_cVoiceBroadcastData;


//flash.c
//FLASH��д״̬
extern unsigned char g_cFLASHWrStatus;
extern unsigned char g_cFLASHRdStatus;

//FLASH��д����
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


//������  ���ݻ���
extern unsigned char g_cEraseSectionBuff[1024];
//extern unsigned char g_cEraseSectionBuff[16];



#endif
