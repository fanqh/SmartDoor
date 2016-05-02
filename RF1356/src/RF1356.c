/*
* Copyright (c) 2008,上海图安信息技术有限公司
* All rights reserved.
*
* 工程名称：1211-C指纹锁项目
* 文件名称：RF1356.c
* 文件标识：无
* 摘    要：13.56读卡相关操作
*
* 当前版本：2.0
* 作    者：汪华红
* 邮    箱：wanghuahong@tooan.cn
* 起始日期：2012年09月19日
* 完成日期：
*
* 编译环境：Keil V4.23.00.0
* 芯片型号：STM32F205VG
*/

#include "RF1356.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"

/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

//#define MAXRLEN               16

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                           0x00    
#define     COMMAND_REG                     0x01    
#define     COM_IEN_REG                     0x02    
#define     DIVL_EN_REG                     0x03    
#define     COM_IRQ_REG                     0x04    
#define     DIV_IRQ_REG                     0x05
#define     ERROR_REG                       0x06    
#define     STATUS1_REG                     0x07    
#define     STATUS2_REG                     0x08    
#define     FIFO_DATA_REG                   0x09
#define     FIFO_LEVEL_REG                  0x0A
#define     WATER_LEVEL_REG                 0x0B
#define     CONTROL_REG                     0x0C
#define     BIT_FRAMING_REG                 0x0D
#define     COLL_REG                        0x0E
#define     RFU0F                           0x0F
// PAGE 1     
#define     RFU10                           0x10
#define     MODE_REG                        0x11
#define     TX_MODE_REG                     0x12
#define     RX_MODE_REG                     0x13
#define     TX_CONTROL_REG                  0x14
#define     TX_AUTO_REG                     0x15
#define     TX_SEL_REG                      0x16
#define     RX_SEL_REG                      0x17
#define     RX_THRESHOLD_REG                0x18
#define     DEMOD_REG                       0x19
#define     RFU1A                           0x1A
#define     RFU1B                           0x1B
#define     MIFARE_REG                      0x1C
#define     RFU1D                           0x1D
#define     RFU1E                           0x1E
#define     SERIAL_SPEED_REG                0x1F
// PAGE 2    
#define     RFU20                           0x20  
#define     CRC_RESULT_REG_M                0x21
#define     CRC_RESULT_REG_L                0x22
#define     RFU23                           0x23
#define     MOD_WIDTH_REG                   0x24
#define     RFU25                           0x25
#define     RF_CFG_REG                      0x26
#define     GSN_REG                         0x27
#define     CW_GS_CFG_REG                   0x28
#define     MOD_GS_CFG_REG                  0x29
#define     TMODE_REG                       0x2A
#define     TPRESCALER_REG                  0x2B
#define     TRELOAD_REG_H                   0x2C
#define     TRELOAD_REG_L                   0x2D
#define     TCOUNTER_VALUE_REG_H            0x2E
#define     TCOUNTER_VALUE_REG_L            0x2F
// PAGE 3      
#define     RFU30                           0x30
#define     TEST_SEL1_REG                   0x31
#define     TEST_SEL2_REG                   0x32
#define     TEST_PIN_EN_REG                 0x33
#define     TEST_PIN_VALUE_REG              0x34
#define     TEST_BUS_REG                    0x35
#define     AUTO_TEST_REG                   0x36
#define     VERSION_REG                     0x37
#define     ANALOG_TEST_REG                 0x38
#define     TEST_DAC1_REG                   0x39  
#define     TEST_DAC2_REG                   0x3A   
#define     TEST_ADC_REG                    0x3B   
#define     RFU3C                           0x3C   
#define     RFU3D                           0x3D   
#define     RFU3E                           0x3E   
#define     RFU3F		            0x3F



/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

#define MAXRLEN               18

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  0x3F







#define  RF1356_SET_CS_HIGH()   	GPIO_SetBits(RFID_CTRL_PORT, RFID_CS_PIN) /*拉高 片选*/
#define  RF1356_SET_CS_LOW()	    GPIO_ResetBits(RFID_CTRL_PORT, RFID_CS_PIN)

#define  RF1356_SET_CLK_HIGH()      GPIO_SetBits(RFID_CTRL_PORT, RFID_CLK_PIN) /*拉高 shizhong*/
#define  RF1356_SET_CLK_LOW()	    GPIO_ResetBits(RFID_CTRL_PORT, RFID_CLK_PIN)

#define  RF1356_SET_MOSI_HIGH()     GPIO_SetBits(RFID_CTRL_PORT, RFID_MOSI_PIN  ) /*拉高 MOSI*/
#define  RF1356_SET_MOSI_LOW()	    GPIO_ResetBits(RFID_CTRL_PORT, RFID_MOSI_PIN)

#define  RF1356_RD_MISO()           GPIO_ReadInputDataBit(RFID_CTRL_PORT, RFID_MISO_PIN)


#define MF_VCCCTRL                              GPIO_Pin_11
#define RFID_BACKLIGHT                          GPIO_Pin_12


void RF1356_MasterInit(void);
void RF1356_SoftSpiWrByte(unsigned char data);
unsigned char RF1356_SoftSpiRdByte(void);
void RF1356_SetBitMask(unsigned char reg,unsigned char mask);
void RF1356_ClearBitMask(unsigned char reg,unsigned char mask);
void RF1356_PcdAntennaOn(void);
void RF1356_PcdAntennaOff(void);
unsigned char RF1356_PcdComMF522(  unsigned char Command, 
                                   unsigned char *pInData, 
                                   unsigned char InLenByte,
                                   unsigned char *pOutData, 
                                   unsigned int  *pOutLenBit);
unsigned char RF1356_PcdRequest(unsigned char req_code,unsigned char *pTagType);
unsigned char RF1356_PcdAnticoll(unsigned char *pSnr);
void RF1356_CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
unsigned char RF1356_PcdSelect(unsigned char *pSnr);
unsigned char RF1356_PcdAnticoll(unsigned char *pSnr);
void RF1356_CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
unsigned char RF1356_PcdSelect(unsigned char *pSnr);
unsigned char RF1356_PcdHalt(void);


void RF1356_MasterInit(void)
{//使用软件SPI

//独立IO口
    GPIO_InitTypeDef GPIO_InitStructure;                    //定义IO配置结构体
    //SPI_I2S_DeInit(SPI2);
    //SPI_Cmd(SPI2, DISABLE);
    //GPIO_SetBits(GPIOB,GPIO_Pin_12);
 
	
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);  //clock enbale
	GPIO_ResetBits(RFID_CTRL_PORT, RFID_MOSI_PIN | RFID_CLK_PIN );
    //GPIO B  输出IO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIO_InitStructure.GPIO_Pin  = RFID_MOSI_PIN  | RFID_CLK_PIN | RFID_CS_PIN | RFID_RESET_PIN;//SPI CLK MOSI输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO速率
	GPIO_Init(RFID_CTRL_PORT,&GPIO_InitStructure);                   //设置IO
    
	
	RF1356_SET_CS_HIGH();
	GPIO_ResetBits(RFID_CTRL_PORT, RFID_MOSI_PIN | RFID_CLK_PIN );

    //GPIO B  输入IO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //输出模式
	GPIO_InitStructure.GPIO_Pin  = RFID_MISO_PIN;            //SPI MISO 输入                                   
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO速率
	GPIO_Init(RFID_CTRL_PORT,&GPIO_InitStructure);                   //设置IO
	
    
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //输出模式
//	GPIO_InitStructure.GPIO_Pin  =  CARD_IRQ_PIN ;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
//	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;       //无上下拉电阻
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;       //IO速率
//	GPIO_Init(CARD_IRQ_PORT, &GPIO_InitStructure);                   //设置IO
	//GPIO_ResetBits(CARD_IRQ_PORT, CARD_IRQ_PIN);
}


void RF1356_SoftSpiWrByte(unsigned char data)
{
    unsigned char i;
    
    for(i=0;i<8;i++)
    {

        RF1356_SET_CLK_LOW();
        if(data & 0x80)
        {
            RF1356_SET_MOSI_HIGH();
        }
        else
        {
            RF1356_SET_MOSI_LOW();
        }
        data = data << 1;
        RF1356_SET_CLK_HIGH();
        delay_us(5);
    }
    RF1356_SET_CLK_LOW();
    RF1356_SET_MOSI_LOW();
}

unsigned char RF1356_SoftSpiRdByte(void)
{
    unsigned char i;
    unsigned char data=0x00;
    
    for(i=0;i<8;i++)
    {

        RF1356_SET_CLK_LOW();
        delay_us(5);
        RF1356_SET_CLK_HIGH();        
        data = data << 1;
        if(RF1356_RD_MISO())
        {
            data = data | 0x01;
            //GPIO_SetBits(BEEP_CTRL_PORT,BEEP_CTRL_PIN);//输出高
        }        
    }
    RF1356_SET_CLK_LOW();
    return data;
}

#if 0
void RF1356_SpiCycle(unsigned char *input,unsigned char *output,unsigned int len)
{
	unsigned int i;
	//Spi_CS_Set_Low();
	for(i=0;i<len;i++)
  	{
       
       
  		SPI2->DR = input[i];
		while((SPI2->SR & SPI_I2S_FLAG_RXNE ) == RESET);//等待接收完成
		SPI2->SR &=RX_CLEAR;						//清除标志可以不要
		if(output != NULL)
		{
		    output[i]= SPI2->DR;
		}
		
		while((SPI2->SR & SPI_I2S_FLAG_TXE ) == RESET);//等待传输完成
		SPI2->SR &= TX_CLEAR;						//清除标志可以不要
	}
	while((SPI2->SR & SPI_I2S_FLAG_BSY ) != RESET);	//等待接收完成
	//Spi_CS_Set_High();
}
#endif



/***************************************************************
* 函数名:RF1356_MasterWriteData（）
* MCU通过SPI写数据
* 输入参数: void
* 输出参数: 无
* 返回:     void
*****************************************************************/
void RF1356_MasterWriteData(unsigned char addr,unsigned char wrdata)
{
//    unsigned char buff[3];
    RF1356_SET_CS_LOW();
    delay_us(5);

    addr = ((addr<<1) & 0x7E);
    RF1356_SoftSpiWrByte(addr);
    RF1356_SoftSpiWrByte(wrdata); 
    RF1356_SET_CS_HIGH();//Disable CS	
}

//spi读操作
unsigned char RF1356_MasterReadData(unsigned char addr)
{
    //unsigned char buff[2];
    //unsigned char rddata[2];
    unsigned char rddata;

    RF1356_SET_CS_LOW();//Enable CS
    delay_us(5);
    
    addr = ((addr<<1) & 0x7E) | 0x80;
    RF1356_SoftSpiWrByte(addr);
    rddata = RF1356_SoftSpiRdByte();
    RF1356_SET_CS_HIGH();//Disable CS
    return(rddata);	
}



unsigned char RF1356_MasterReadData2(unsigned char addr,unsigned char *p)
{
    //unsigned char buff[2];
    //unsigned char rddata[2];
    unsigned char rddata;

    RF1356_SET_CS_LOW();//Enable CS
    delay_us(5);
    
    addr = ((addr<<1) & 0x7E) | 0x80;
    RF1356_SoftSpiWrByte(addr);
    rddata = RF1356_SoftSpiRdByte();
    *p=rddata;
    RF1356_SET_CS_HIGH();//Disable CS
    return 1;	
}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void RF1356_SetBitMask(unsigned char reg,unsigned char mask)  
{
    unsigned char tmp = 0x00;
    tmp = RF1356_MasterReadData(reg);
    RF1356_MasterWriteData(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void RF1356_ClearBitMask(unsigned char reg,unsigned char mask)  
{
    unsigned char tmp = 0x00;
    tmp = RF1356_MasterReadData(reg);
    RF1356_MasterWriteData(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void RF1356_PcdAntennaOn(void)
{
    unsigned char i;
    i = RF1356_MasterReadData(TX_CONTROL_REG);
    if (!(i & 0x03))
    {
        RF1356_SetBitMask(TX_CONTROL_REG, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void RF1356_PcdAntennaOff(void)
{
    RF1356_ClearBitMask(TX_CONTROL_REG, 0x03);
}


/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
unsigned char RF1356_PcdComMF522(  unsigned char Command, 
                                   unsigned char *pInData, 
                                   unsigned char InLenByte,
                                   unsigned char *pOutData, 
                                   unsigned int  *pOutLenBit)
{
    unsigned char status = MI_ERR;
    //unsigned char irqEn   = 0x00;上升沿
    unsigned char irqEn   = 0x80; //下降沿
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    
    if(Command == PCD_AUTHENT)
    {
        irqEn   = 0x12;
        waitFor = 0x10;
    }
    else if(Command == PCD_TRANSCEIVE)
    {
        irqEn   = 0x77;
        waitFor = 0x30;
    }
   
    RF1356_MasterWriteData(COM_IEN_REG,irqEn|0x80);
    RF1356_ClearBitMask(COM_IRQ_REG,0x80);
    RF1356_MasterWriteData(COMMAND_REG,PCD_IDLE);
    RF1356_SetBitMask(FIFO_LEVEL_REG,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
        RF1356_MasterWriteData(FIFO_DATA_REG, pInData[i]);    
    }
    RF1356_MasterWriteData(COMMAND_REG, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    
        RF1356_SetBitMask(BIT_FRAMING_REG,0x80);  
    }
    
 //   i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
 //   i = 2000;
	i = 1000;
    do 
    {
         n = RF1356_MasterReadData(COM_IRQ_REG);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    RF1356_ClearBitMask(BIT_FRAMING_REG,0x80);
	      
    if (i!=0)
    {    
         if(!(RF1356_MasterReadData(ERROR_REG)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   
                 status = MI_NOTAGERR;   
             }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = RF1356_MasterReadData(FIFO_LEVEL_REG);
              	lastBits = RF1356_MasterReadData(CONTROL_REG) & 0x07;
                if (lastBits)
                {   
                    *pOutLenBit = (n-1)*8 + lastBits;   
                }
                else
                {   
                    *pOutLenBit = n*8;   
                }
                
                if (n == 0)
                {   
                    n = 1;    
                }
                
                if (n > MAXRLEN)
                {   
                    n = MAXRLEN;   
                }
                for (i=0; i<n; i++)
                {   
                    pOutData[i] = RF1356_MasterReadData(FIFO_DATA_REG);    
                }
            }
         }
         else
         {   
             status = MI_ERR;   
         }
        
   }
   

   RF1356_SetBitMask(CONTROL_REG,0x80);           // stop timer now
   RF1356_MasterWriteData(COMMAND_REG,PCD_IDLE); 
   return status;
}


unsigned char RF1356_RC523Init(void)
{
    
 //   RF1356_MasterInit();
  //  delay_ms(5);
    RF1356_SET_RESET_HIGH();//RST = 1
    delay_us(5);
    RF1356_SET_RESET_LOW();//RST = 0
    delay_us(5);
    RF1356_SET_RESET_HIGH();//RST = 1
    delay_us(5);
    RF1356_MasterWriteData(COMMAND_REG,PCD_RESETPHASE);
    delay_us(5);
	
    RF1356_MasterWriteData(MODE_REG,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    RF1356_MasterWriteData(TRELOAD_REG_L,30);           
    RF1356_MasterWriteData(TRELOAD_REG_H,0);
    RF1356_MasterWriteData(TMODE_REG,0x8D);
    RF1356_MasterWriteData(TPRESCALER_REG,0x3E);
    RF1356_MasterWriteData(TX_AUTO_REG,0x40);  
    
    RF1356_PcdAntennaOff();//开启天线
    RF1356_PcdAntennaOn();//关闭天线
    
    //ISO 类型 A
    RF1356_ClearBitMask(STATUS2_REG,0x08);
    RF1356_MasterWriteData(MODE_REG,0x3D);//3F    
    RF1356_MasterWriteData(RX_SEL_REG,0x86);//84    
    RF1356_MasterWriteData(RF_CFG_REG,0x7F);   //4F    
    RF1356_MasterWriteData(TRELOAD_REG_L,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
    RF1356_MasterWriteData(TRELOAD_REG_H,0);
    RF1356_MasterWriteData(TMODE_REG,0x8D);//
    RF1356_MasterWriteData(TPrescalerReg,0x3E);
    delay_us(10);
    //RF1356_PcdAntennaOn();//天线
	RF1356_PcdAntennaOff();
    
    return MI_OK;
}


/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
unsigned char RF1356_PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   unsigned char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 
   
   RF1356_ClearBitMask(STATUS2_REG,0x08);
   RF1356_MasterWriteData(BIT_FRAMING_REG,0x07);
   RF1356_SetBitMask(TX_CONTROL_REG,0x03);
 
   ucComMF522Buf[0] = req_code;

   status = RF1356_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

   if ((status == MI_OK) && (unLen == 0x10))
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   
       status = MI_ERR;   
   }
   
   return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
unsigned char RF1356_PcdAnticoll(unsigned char *pSnr)
{
    unsigned char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    RF1356_ClearBitMask(STATUS2_REG,0x08);
    RF1356_MasterWriteData(BIT_FRAMING_REG,0x00);
    RF1356_ClearBitMask(COLL_REG,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = RF1356_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   
             status = MI_ERR;    
         }
    }
    
    RF1356_SetBitMask(COLL_REG,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void RF1356_CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    RF1356_ClearBitMask(DIV_IRQ_REG,0x04);
    RF1356_MasterWriteData(COMMAND_REG,PCD_IDLE);
    RF1356_SetBitMask(WATER_LEVEL_REG,0x80);
    for (i=0; i<len; i++)
    {   
        RF1356_MasterWriteData(FIFO_DATA_REG, *(pIndata+i));   
    }
    RF1356_MasterWriteData(COMMAND_REG, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = RF1356_MasterReadData(DIV_IRQ_REG);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = RF1356_MasterReadData(CRC_RESULT_REG_L);
    pOutData[1] = RF1356_MasterReadData(CRC_RESULT_REG_M);
}

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
unsigned char RF1356_PcdSelect(unsigned char *pSnr)
{
    unsigned char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    RF1356_CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    RF1356_ClearBitMask(STATUS2_REG,0x08);

    status = RF1356_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
        status = MI_OK;  
    }
    else
    { 
        status = MI_ERR;    
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
unsigned char RF1356_PcdHalt(void)
{
    //unsigned char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    RF1356_CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    RF1356_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    //RF1356_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

#if 0
	#define RFID1356_READ_CARD_OK                       0x02
	#define RFID1356_DISABLE_CARD_RX                    0x01
	#define RFID1356_ENABLE_CARD_RX                     0x00
	#define CARD_MAX_NUM                     			100			//卡最大用户数量
	#define FINGER_MAX_NUM                     			100			//指纹最大用户数量

	#define ONE_CARD_SIZE                     			8			//一个卡用户字节数

	#define BEEP_DI(num,di_time,di_mod)               	{g_cBeepDiAllTime = di_time; g_cBeepDiNum = num; g_cBeepDiTime = 0;g_cBeepMod = di_mod;} 
	#define BEEP_DI_SHORT_TIME                 	20//40
	#define BEEP_DI_LONG_TIME                  	100//100
	#define	BEEP_DI_FINGER_TIME	 				50
	#define BEEP_MOD_KEY					 	0x01	 					//按键响声音
#endif

int16_t RF1356_GetCard(uint8_t *ptr)
{
    unsigned char temp;
//    RF1356_PcdAntennaOn();
    
//	RF1356_MasterInit();
	temp = RF1356_PcdRequest(PICC_REQIDL, g_cRfidRxBuff); //PICC_REQIDL  PICC_REQALL
	if(temp == MI_OK)
	{
		temp = RF1356_PcdAnticoll(g_cRfidRxBuff);
		if(temp == MI_OK)
		{
			temp = RF1356_PcdSelect(g_cRfidRxBuff);
			//return 1;
			//DELAY_MS(2000);
		  //  g_cGetCardStatus = RFID1356_READ_CARD_OK;
		 //   if(temp == MI_OK)
			{
		  //      RF1356_PcdHalt();
			}
		}
	}
	RF1356_PcdAntennaOff();
	memcpy(ptr,g_cRfidRxBuff,4);
    return temp;
}

#if 1
int16_t RTC_WakeUp_GetCard(uint8_t *ptr)
{
    unsigned char temp = MI_ERR;
    //RF1356_RC523Init();
	//DELAY_MS(100);
    //BEEP_DI(1,BEEP_DI_SHORT_TIME);
 	//	RF1356_PcdAntennaOff();
	//	DELAY_MS(1);
	RF1356_PcdAntennaOn();
//	RF1356_MasterInit();
	//DELAY_MS(100);
	temp = RF1356_PcdRequest(PICC_REQIDL, g_cRfidRxBuff); //PICC_REQIDL  PICC_REQALL
	if(temp == MI_OK)
	{
		temp = RF1356_PcdAnticoll(g_cRfidRxBuff);
		if(temp == MI_OK)
		{
			temp = RF1356_PcdSelect(g_cRfidRxBuff);
			if(temp == MI_OK)
			{
				RF1356_PcdHalt();
				memcpy(ptr,g_cRfidRxBuff,4);
			}
		}
	}
	RF1356_PcdAntennaOff();
	return temp;
}


void Rf1356_InitGetCard(void)
{
    unsigned char temp;
    RF1356_RC523Init();
	//RF1356_MasterInit();
	//DELAY_MS(100);
	//BEEP_DI(1,BEEP_DI_SHORT_TIME);
	temp = RF1356_PcdRequest(PICC_REQIDL, g_cRfidRxBuff);
	if(temp == MI_OK)
	{
		temp = RF1356_PcdAnticoll(g_cRfidRxBuff);
		if(temp == MI_OK)
		{
			temp = RF1356_PcdSelect(g_cRfidRxBuff);
			//BEEP_DI(1,BEEP_DI_SHORT_TIME);
			//LIGHT_GREEN_ON();
			if(temp == MI_OK)
			{
				RF1356_PcdHalt();
			}
		}
	}
}
#endif

void RF1356_SleepCheckCard(void)
{
    unsigned char i;

    //RF1356_SET_RESET_HIGH();//RST = 1
	GPIO_SetBits(GPIOA, RFID_RESET_PIN); //RST = 1
	delay_us(10);
   	RF1356_MasterWriteData(COMMAND_REG,PCD_RESETPHASE);
	//DELAY_MS(10);
	RF1356_PcdAntennaOn();//天线


    for(i=0;i<3;i++)
    {
        if(RF1356_MasterReadData(COMMAND_REG) == 0x20)
        {
            break;
        }
    }
    //g_sRfidCaptureCount = 0;
    RF1356_MasterWriteData(TX_CONTROL_REG,0x83);
    delay_us(1);
	GPIO_ResetBits(GPIOA, RFID_RESET_PIN);	  //RST = 0;
}

void card_irq_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //输出模式
	GPIO_InitStructure.GPIO_Pin  =  CARD_IRQ_PIN ;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //IO类型，推挽类型
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;       //pull up
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;       //IO速率
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);  //clock enbale
	GPIO_Init(CARD_IRQ_PORT, &GPIO_InitStructure);                   //设置IO
}

uint8_t card_irq_status(void)
{
    return GPIO_ReadInputDataBit(CARD_IRQ_PORT,CARD_IRQ_PIN);
}


