#ifndef __SPI_H_
#define __SPI_H_
#include "stm32f0xx_conf.h"

#define SPI_LIB  0

//void Spi_Init(void);
void Enable_CS(void);
void Disable_CS(void);
//uint8_t Spi_Master_WriteRead(uint8_t data);

void SpiMsterGpioInit(void);
void SPICmd8bit(uint8_t WrPara)	;
uint8_t SPIRead8bit(void);

#endif


