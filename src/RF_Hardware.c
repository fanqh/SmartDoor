#include"RF_Hardware.h"

#include "Delay.h"
#include <stm32f0xx.h>
#include "stm32f0xx_gpio.h"

//SPI总线
#define  RF_SPI_GPIO_PORT		GPIOB
#define  RF_SPI_CLK				GPIO_Pin_3
#define  RF_SPI_MISO			GPIO_Pin_4
#define  RF_SPI_MOSI			GPIO_Pin_5
#define  RF_SPI_CS			GPIO_Pin_6

#define  RF_RESET_PIN			GPIO_Pin_7
#define  RF_REDE_GPIO_PORT		GPIOB

#define  RF_Spi_Clk_High()      GPIO_SetBits(RF_SPI_GPIO_PORT, RF_SPI_CLK)/*拉高 复位*/
#define  RF_Spi_Clk_Low()	    GPIO_ResetBits(RF_SPI_GPIO_PORT, RF_SPI_CLK)

#define  RF_Spi_Mosi_High()     GPIO_SetBits(RF_SPI_GPIO_PORT, RF_SPI_MOSI)/*拉高 复位*/
#define  RF_Spi_Mosi_Low()	    GPIO_ResetBits(RF_SPI_GPIO_PORT, RF_SPI_MOSI)

#define  RF_Spi_Cs_High()       GPIO_SetBits(RF_SPI_GPIO_PORT, RF_SPI_CS)
#define  RF_Spi_Cs_Low()	    GPIO_ResetBits(RF_SPI_GPIO_PORT, RF_SPI_CS)


#define  RF_Spi_Miso_Read()          RF_SPI_GPIO_PORT->IDR & RF_SPI_MISO

void RF_Spi_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;                    //定义IO配置结构体

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIO_InitStructure.GPIO_Pin  = RF_SPI_CLK |RF_SPI_MOSI|RF_SPI_CS;//SPI CLK MOSI输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO速率
	GPIO_Init(RF_SPI_GPIO_PORT,&GPIO_InitStructure);                   //设置IO
    RF_Spi_Clk_Low();
    RF_Spi_Mosi_Low();
    RF_Spi_Cs_High();

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIO_InitStructure.GPIO_Pin  = RF_RESET_PIN;            //复位输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO速率
	GPIO_Init(RF_REDE_GPIO_PORT,&GPIO_InitStructure);  
	RF_Reset_Low();
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //输入模式
	GPIO_InitStructure.GPIO_Pin  = RF_SPI_MISO;            //SPI MISO 输入                                   
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO速率
	GPIO_Init(RF_SPI_GPIO_PORT,&GPIO_InitStructure);                   //设置IO

}
void RF_Reset_High(void)    
{
    GPIO_SetBits(RF_REDE_GPIO_PORT, RF_RESET_PIN);
}

void RF_Reset_Low(void)
{
    GPIO_ResetBits(RF_REDE_GPIO_PORT, RF_RESET_PIN);
}
void RF_SoftSpiWrByte(uint8_t ucData)
{
    uint8_t i;    
    for(i=0;i<8;i++)
    {
        RF_Spi_Clk_Low();
        if(ucData & 0x80)
        {
            RF_Spi_Mosi_High();
        }
        else
        {
            RF_Spi_Mosi_Low();
        }
        ucData = ucData << 1;
        RF_Spi_Clk_High();
        delay_us(8);
    }
    //RF_Spi_Clk_Low();
    RF_Spi_Mosi_Low();
}

uint8_t RF_SoftSpiRdByte(void)
{
    uint8_t i;
    uint8_t ucData=0x00;   
    for(i=0;i<8;i++)
    {
        RF_Spi_Clk_Low();
 //       delay_us(8);
   //     RF_Spi_Clk_High();       
        ucData = ucData << 1;
        if(RF_Spi_Miso_Read())
        {
            ucData = ucData | 0x01;
        } 
		RF_Spi_Clk_High();  
//  delay_us(8);        
    }
   // RF_Spi_Clk_Low();
    return ucData;
}
void RF_MasterWriteData(uint8_t addr,uint8_t wrdata)
{
    RF_Spi_Cs_Low();
    delay_us(8);

    addr = ((addr<<1) & 0x7E);
    RF_SoftSpiWrByte(addr);
    RF_SoftSpiWrByte(wrdata);  
    RF_Spi_Cs_High();;//Disable CS	
}

uint8_t RF_MasterReadData(uint8_t addr)
{
    unsigned char rddata;

    RF_Spi_Cs_Low();;//Enable CS
    delay_us(8);
    
    addr = ((addr<<1) & 0x7E) | 0x80;
    RF_SoftSpiWrByte(addr);
    rddata = RF_SoftSpiRdByte();
    RF_Spi_Cs_High();//Disable CS
    return(rddata);	
}




