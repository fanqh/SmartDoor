/*
* Copyright (c) 2012,�Ϻ�ͼ����Ϣ�������޹�˾
* All rights reserved.
*
* �������ƣ�1211-Cָ������Ŀ
* �ļ����ƣ�global.c
* �ļ���ʶ����
* ժ    Ҫ��ȫ�ֱ�������Դ�ļ�
*
* ��ǰ�汾��2.0
* ��    �ߣ�������
* ��    �䣺wanghuahong@tooan.cn
* ��ʼ���ڣ�2012��09��30��
* ������ڣ�
*
* ���뻷����Keil V4.23.00.0
* оƬ�ͺţ�STM32F205VG
*/

#include "RF1356.h"


//finger.c

//ָ��Ӧ��ͬ����EEPROMһ�����ֳɶ�����СӦ�ã�ͨ��g_cFingerOperatingStatus���ݵ�ǰ�ľ���Ӧ��
//����ʵ���ڵ����ȼ�ѭ����ִ�С�g_cFingerOperatingDataBuff[0~2]�����ݽ�������ݣ�
//g_cFingerOperatingDataBuff[0] ��ʾ��ǰӦ�õĳɹ���ʧ�ܻ�������������BUFF���ݱ�ʾ�����������ݡ�
//g_cFingerOperatingDataBuff[1]
//g_cFingerOperatingDataBuff[2]
//0 1 2 �ӵײ㷵��
//g_cFingerOperatingDataBuff[3];//�û����
//g_cFingerOperatingDataBuff[4]	//�û����
//g_cFingerOperatingDataBuff[5];//�û�Ȩ��
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
unsigned char g_ckeyState[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //0 1 2 3 4 5 6 7 8 9 * # S(��λ��) ������*��  ������#�� 
unsigned char g_ckeyTime[15]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char g_cKeyCurrentValue=0xFF;//0:0  1:1  2:2  3:3  4:4  5:5  6:6  7:7  8:8  9:9  10:*  11:#  12:��λ��  13:������*�� 14:������#��
unsigned char g_cKeyTouchCount=0;
unsigned char g_cKeyTouchBuff[4];
unsigned short g_cKeyResetTime;
unsigned char g_cTouchKeyStart = 0;
unsigned char g_cKeyUpKeyValue = 0xFF;


//ϵͳ��������
unsigned char g_cDeviceUseOfficeFlag=0;
unsigned char g_cDeviceLockFlag=0;		//�豸������ʶ	 ���Ӳ���ʧЧ
//unsigned char g_cDeviceStatus=0x01;//��ǰ�豸����״̬,��λ��ʹ�����ڸ��ϵ�״̬

unsigned char g_cDevicePowerOnTestFlag=0;//�豸�ϵ�ֱ�ӽ�����Խ׶α�ʶ����λ��1�������ϻ��������̡�
unsigned short g_sDeviceTestCount=0;
unsigned char g_cDeviceAlwaysOpenFlag=0;//�ų�����־
unsigned char g_cTempErrorTime;//��ʱ�������
unsigned char g_cOpenDoorCheckBuff[6];	   //ÿ����֤���ŵ�ID���Լ�����
unsigned char g_cSafeModeCheckOptions = 0;//��ȷ��������Ĵ���
unsigned char g_cRTC_Wakeup_Flag = 0;
volatile unsigned char g_c200MS_GetCard = 0;
unsigned char g_c200MS_ReadKeyFlag = 0;
unsigned char g_c200MS_ReadKeyTimeTip = 0;	   //����������ʱ��Ƭ
unsigned char g_cMeasurePlusOver = 0;
unsigned short g_cUsrnum = 0;
unsigned char g_cCardnum = 0;



unsigned char g_cDeviceCheckMode=0;//0������֤ģʽ��1��˫����֤��2��������֤
unsigned char g_cOpenDoorErrorCount=0xFF;//���Ŵ������������
unsigned char g_cUserAmount=0;//�û�����
unsigned char g_cAdminAmount=0;//����Ա����
unsigned char g_cPerUserAmount=0;//��פ�û�����
unsigned char g_cTempUserAmount=0;//��ʱ�û�����




//EEPROM��д״̬
unsigned char g_cEepromWrStatus = 0xFF;
unsigned char g_cEepromRdStatus = 0xFF;

//EEPROM��д����
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

//��������
unsigned char g_cAlarmFlag=0;
unsigned int g_cAlarmCount=0;
unsigned short g_cAlarmTimerCount;

//ADC��ѹ���
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
unsigned char g_cMotorDriveStatus=0xFF;//���ת��״̬��������ת������ת
unsigned int g_sMotorDriveCount;//���ڿ����ŵ�ʱ����м�ʱ
unsigned char g_cMotorDriveStep;
unsigned char g_cOpenDoor_Mod = 0xFF;	//0xF0 ����ģʽ    0xFF  ����ģʽ	  0x00  ����ģʽ




//card.c
#define RFID1356_READ_CARD_OK                       0x02
#define RFID1356_DISABLE_CARD_RX                    0x01
#define RFID1356_ENABLE_CARD_RX                     0x00
volatile unsigned char g_cGetCardStatus = RFID1356_ENABLE_CARD_RX;
unsigned char g_cCardUserAmount = 0;
unsigned char g_cRfidRxBuff[20];
unsigned char g_cInitStatusEnRdCardFlag;

//���뿪��ʹ�û�����
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
//FLASH��д״̬
unsigned char g_cFLASHWrStatus = 0xFF; 
unsigned char g_cFLASHRdStatus = 0xFF;

//FLASH��д����
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


//������  ���ݻ���
unsigned char g_cEraseSectionBuff[1024];
//unsigned char g_cEraseSectionBuff[16];

