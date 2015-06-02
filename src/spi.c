#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "spi.h"
#include "delay.h"



void Enable_CS(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_6);
}
void Disable_CS(void)  /////!!!!!!!异常为什么操作过后不能拉高
{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
	//GPIO_ResetBits(GPIOB, GPIO_Pin_6);
}

#if SPI_LIB
void Spi_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3|GPIO_PinSource4|GPIO_PinSource5, GPIO_AF_0); 
	//PB3,PB5  mosi sclk    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_3|GPIO_Pin_4;		           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//pb6 cs
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

/*--------------- Reset SPI init structure parameters values -----------------*/
  /* Initialize the SPI_Direction member */
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  /* Initialize the SPI_Mode member */
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  /* Initialize the SPI_DataSize member */
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  /* Initialize the SPI_CPOL member */
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  /* Initialize the SPI_CPHA member */
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  /* Initialize the SPI_NSS member */
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  /* Initialize the SPI_BaudRatePrescaler member */
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  /* Initialize the SPI_FirstBit member */
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	
	
  SPI_Init(SPI1, &SPI_InitStruct);
  SPI_Cmd(SPI1, ENABLE);
	Disable_CS();
//Enable_CS();
}


uint8_t Spi_Master_WriteRead(uint8_t data)
{
	uint16_t retry = 0;
	uint8_t temp = 0;

	Enable_CS();
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)==RESET)
	{
		retry ++;
		if(retry>200)
			return 0;
	}	
	SPI_SendData8(SPI1, data);
	retry = 0;
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)==RESET)
	{
		retry ++;														
		if(retry>200)
			return 0;
	}
	temp = 	SPI_ReceiveData8(SPI1);
	Disable_CS();
	return 	temp;		
}
#else

#define SPI_PIN_SCK  GPIO_Pin_3
#define SPI_PIN_MISO GPIO_Pin_4
#define SPI_PIN_MOSI GPIO_Pin_5
#define SPI_PIN_CS 	 GPIO_Pin_6
#define SPI_PORT   GPIOB

//#define  IRQ			   PBin(2)


void SpiMsterGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
						
	//gpio clck enable
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	  /* Configure SPIy pins: SCK, MOSI ---------------------------------*/
   GPIO_InitStructure.GPIO_Pin = SPI_PIN_MOSI | SPI_PIN_SCK | SPI_PIN_CS;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(SPI_PORT, &GPIO_InitStructure);

	
   //MISO
   GPIO_InitStructure.GPIO_Pin = SPI_PIN_MISO;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_Init(SPI_PORT, &GPIO_InitStructure);
	
	 GPIO_SetBits(SPI_PORT, SPI_PIN_CS);

}


/**********************************************************
**Name:     SPICmd8bit
**Function: SPI Write one byte
**Input:    WrPara	spi :SPI_1 SPI_2
**Output:   none
**note:     use for burst mode
**********************************************************/
void SPICmd8bit(uint8_t WrPara)	//SPI_I2S_SendData(SPI2, value);
{
  uint8_t bitcnt;

//	GPIO_ResetBits(SPI_PORT,SPI_PIN_CS);
  delay_us(5);
  for(bitcnt=8; bitcnt!=0; bitcnt--)
  {
    GPIO_ResetBits(SPI_PORT,SPI_PIN_SCK);
    if(WrPara&0x80)
      GPIO_SetBits(SPI_PORT,SPI_PIN_MOSI);
    else
      GPIO_ResetBits(SPI_PORT,SPI_PIN_MOSI);
		
		delay_us(5); 	
		WrPara <<= 1;
		GPIO_SetBits(SPI_PORT,SPI_PIN_SCK);
		delay_us(5);
  }

  GPIO_ResetBits(SPI_PORT,SPI_PIN_SCK);
  GPIO_SetBits(SPI_PORT,SPI_PIN_MOSI);
 // delay_us(5);
}

/**********************************************************
**Name:     SPIRead8bit
**Function: SPI Read one byte
**Input:    None
**Output:   result byte
**Note:     use for burst mode
**********************************************************/
uint8_t SPIRead8bit(void)	  // status = SPI_I2S_ReceiveData(SPI2);
{
 uint8_t RdPara = 0;
 uint8_t bitcnt;

	//GPIO_ResetBits(SPI_PORT, SPI_PIN_CS);
  delay_us(5);                                                //Read one byte data from FIFO, MOSI hold to High
  for(bitcnt=8; bitcnt!=0; bitcnt--)
  {
    GPIO_ResetBits(SPI_PORT, SPI_PIN_SCK);
		delay_us(5);
    RdPara <<= 1;
    GPIO_SetBits(SPI_PORT,SPI_PIN_SCK);
		delay_us(5);
    if(GPIO_ReadInputDataBit(SPI_PORT, SPI_PIN_MISO)==Bit_SET)
      RdPara |= 0x01;
    else
      RdPara |= 0x00;
  }
  GPIO_ResetBits(SPI_PORT,SPI_PIN_SCK);
//  delay_us(5);
  return(RdPara);
}
#endif


