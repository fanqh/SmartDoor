#ifndef _RF_HARDWARE_H
#define _RF_HARDWARE_H

//#include "aeplatform.h"
#include "stm32f0xx.h"
/*****************************
��������:RF_Spi_Config
����:
��ʼ��ˢ��SPI IO
����:
��
����ֵ:��
******************************/
void RF_Spi_Config(void);

/*****************************
��������:RF_Reset_High
����:
ʹ��ϵͳ
����:
��
����ֵ:��
******************************/
void RF_Reset_High(void)    ;


/*****************************
��������:RF_Reset_Low
����:
��λϵͳ
����:
��
����ֵ:��
******************************/
void RF_Reset_Low(void);

/*****************************
��������:RF_SoftSpiWrByte
����:
��RC522оƬд��һ���ֽ�
����:
ucDataҪд����ֽ�
����ֵ:��
******************************/
void RF_SoftSpiWrByte(uint8_t ucData);



/*****************************
��������:RF_SoftSpiRdByte
����:
��RC522оƬ����һ���ֽ�
����:
��
����ֵ:��RC522���ص�һ���ֽ�
******************************/
uint8_t RF_SoftSpiRdByte(void);


/****************************
��������:RF_MasterWriteData
����:MCUͨ��SPIд����
����: addr :RC522�ļĴ�����ַ
      ucData:��RC522�ļĴ�����ַ�ж���������
����ֵ:  void
****************************/
void RF_MasterWriteData(uint8_t addr,uint8_t ucData);

/***************************
��������:RF_MasterReadData
����:MCUͨ��SPI��ĳ����ַ������
����: addr :RCC522�Ĵ�����ַ
����ֵ: �Ĵ����е�����
**************************/
uint8_t RF_MasterReadData(uint8_t addr);

#endif








