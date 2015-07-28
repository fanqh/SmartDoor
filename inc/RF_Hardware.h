#ifndef _RF_HARDWARE_H
#define _RF_HARDWARE_H

//#include "aeplatform.h"
#include "stm32f0xx.h"
/*****************************
函数名称:RF_Spi_Config
功能:
初始化刷卡SPI IO
参数:
无
返回值:无
******************************/
void RF_Spi_Config(void);

/*****************************
函数名称:RF_Reset_High
功能:
使能系统
参数:
无
返回值:无
******************************/
void RF_Reset_High(void)    ;


/*****************************
函数名称:RF_Reset_Low
功能:
复位系统
参数:
无
返回值:无
******************************/
void RF_Reset_Low(void);

/*****************************
函数名称:RF_SoftSpiWrByte
功能:
向RC522芯片写入一个字节
参数:
ucData要写入的字节
返回值:无
******************************/
void RF_SoftSpiWrByte(uint8_t ucData);



/*****************************
函数名称:RF_SoftSpiRdByte
功能:
从RC522芯片读出一个字节
参数:
无
返回值:从RC522返回的一个字节
******************************/
uint8_t RF_SoftSpiRdByte(void);


/****************************
函数名称:RF_MasterWriteData
功能:MCU通过SPI写数据
参数: addr :RC522的寄存器地址
      ucData:从RC522的寄存器地址中读出的数据
返回值:  void
****************************/
void RF_MasterWriteData(uint8_t addr,uint8_t ucData);

/***************************
函数名称:RF_MasterReadData
功能:MCU通过SPI读某个地址的数据
参数: addr :RCC522寄存器地址
返回值: 寄存器中的数据
**************************/
uint8_t RF_MasterReadData(uint8_t addr);

#endif








