#ifndef _RF_H
#define _RF_H
#include "stm32f0xx.h"
//#include"aeplatform.h"
//和MF522通讯时返回的错误代码
#define MI_OK                          0
#define MI_NOTAGERR                    1
#define MI_ERR                         2


//RFID 1356一些操作状态
#define RFID1356_READ_CARD_OK                       0x02
#define RFID1356_DISABLE_CARD_RX                    0x01
#define RFID1356_ENABLE_CARD_RX                     0x00

//Mifare_One卡片命令字
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
//MF522寄存器定义
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

//MF522 FIFO长度定义
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

#define MAXRLEN               18
//刷卡加密部分宏定义
#define ENCRYPT_ALL_BLOCK  0//定义加密所有扇区
#ifndef ENCRYPT_ALL_BLOCK  //如果没定义加密所有扇区,下面指定需要加密的单独扇区编号0~15
    #define ENCRYPT_BLOCK_NUM           10//如果不定义加密所有扇区,也不定义加密一个扇区,那么对卡不做认证,所有的卡都可以识别
#endif


#define RF_VOL_AVERAGE_MIN_THRESHOLD    50
#define RF_VOL_WAKEUP_PERCENT_MIN    10/100
#define RF_VOL_WAKEUP_PERCENT_MAX    90/100

/*****************************
函数名称:RF_Init
功能:
初始化spi IO和RC522配置
参数:
无
返回值:无
******************************/

void RF_Init(void);


/********************************
功能：置RC522寄存器位
参数：reg:寄存器地址
      mask:置位值，比如置第七位，则为0x80
返回值:无
*********************************/
void RF_SetBitMask(uint8_t reg,uint8_t mask)  ;

/********************************
功能：清RC522寄存器位
参数：reg:寄存器地址
      mask:清位值
返回值:无
*********************************/
void RF_ClearBitMask(uint8_t reg,uint8_t mask) ;

/*************************
功能:开启RC522天线
参数：无
返回值:无
备注:每次启动或关闭天险发射之间应至少有1ms的间隔
*************************/
void RF_PcdAntennaOn(void);

/*************************
功能:关闭RC522天线
参数：无
返回值:无
备注:每次启动或关闭天险发射之间应至少有1ms的间隔
*************************/
void RF_PcdAntennaOff(void);

/***********************************
功    能：通过RC522和ISO14443卡通讯
参数说明：Command[IN]:RC522命令字
          pInData[IN]:通过RC522发送到卡片的数据
          InLenByte[IN]:发送数据的字节长度
          pOutData[OUT]:接收到的卡片返回数据
          *pOutLenBit[OUT]:返回数据的位长度
返回值:    MI_OK     成功返回
           MI_NOTAGERR    通信不成功
           MI_ERR    错误返回
**************************************/
uint8_t RF_PcdComMF522(  uint8_t Command, 
                                   uint8_t *pInData, 
                                   uint8_t InLenByte,
                                   uint8_t *pOutData, 
                                   uint8_t  *pOutLenBit);

/**********************************
功    能：寻卡
参数说明:   req_code[IN]:寻卡方式
                    0x52 = 寻感应区内所有符合14443A标准的卡
                    0x26 = 寻未进入休眠状态的卡
            pTagType[OUT]：卡片类型代码
                    0x4400 = Mifare_UltraLight
                    0x0400 = Mifare_One(S50)
                    0x0200 = Mifare_One(S70)
                    0x0800 = Mifare_Pro(X)
                    0x4403 = Mifare_DESFire
返回值:    MI_OK     成功返回
           MI_NOTAGERR    通信不成功
           MI_ERR    错误返回
            
************************************/
uint8_t RF_PcdRequest(uint8_t req_code,uint8_t *pTagType);


/******************************
功    能：防冲撞
参数说明: pSnr[OUT]:卡片序列号，4字节
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*******************************/  
uint8_t RF_PcdAnticoll(uint8_t *pSnr);

/************************************
说  明:用MF522计算CRC16函数
参  数:     pIndata 输入的数据缓冲区
            len 输入的数据长度
            pOutData 输出的数据缓冲区
返回值:无
*************************************/
void RF_CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData);

/************************************
说  明:选定卡片
参  数: pSnr[IN]:卡片序列号，4字节
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/
uint8_t RF_PcdSelect(uint8_t *pSnr);

/************************************
说  明:命令卡片进入休眠状态
参  数:无
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/
uint8_t RF_PcdHalt(void);

/************************************
说  明:获取卡
参  数:
        Card_type[out] 卡的类型,
                        0:m1 s50卡
                        1:m1 s70卡
        Card_num[out]  卡号
                        (4字节卡号)
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/
uint8_t RF_GetCard(uint8_t *Card_type,uint8_t *Card_num);
/************************************
说  明: 验证卡片密码
参  数: 
        auth_mode[IN]: 密码验证模式
                 0x60 验证A秘钥
                 0x61 验证B秘钥
        addr:块地址
        pKey[IN]:密码
        pSnr[IN]:卡片序列号，4字节
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/

uint8_t RF_PcdAuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *Psnr);

/************************************
说  明: 读取M1卡一块的数据
参  数: 
        addr:块地址
        pData[OUT]:读出的数据，16字节
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/

uint8_t RF_PcdRead(uint8_t addr,uint8_t *pData);

/************************************
说  明: 往M1卡某扇区块3地址中重新写入密码
参  数: 
        
        auth_mode[IN]: 密码验证模式
                 0x60 验证A秘钥PICC_AUTHENT1A
                 0x61 验证B秘钥PICC_AUTHENT1B
        addr:块地址(把卡默认扇区块从0开始编号，例如S50卡，共有0~63块)
            addr=3+n*4;
        pucKey[IN]::要要验证的密码
        pucData[OUT]:重新写入的密码。
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/
uint8_t RF_RelaseBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData);

/************************************
说  明:从M1卡中某块地址中重新读出数据
参  数:         
        auth_mode[IN]: 密码验证模式
                 0x60 验证A秘钥PICC_AUTHENT1A
                 0x61 验证B秘钥PICC_AUTHENT1B
        addr:块地址(把卡默认扇区块从0开始编号，例如S50卡，共有0~63块)
             addr=n;
        pucKey[IN]: 要验证的密码,(该密码必须和卡的密码一致，否侧校验不成功，不能读取数据)
        pucData[OUT]:输出的块字节数据，共16字节。
返回值: MI_OK     成功返回
        MI_NOTAGERR    通信不成功
        MI_ERR    错误返回
*************************************/
uint8_t RF_ReadBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData);

/***********************************
说明：Tx1，Tx2上将会发送13.56MHZ的载波数据
************************************/
void RF_TurnON_TX_Driver_Data(void);

/***********************************
RF Reset and low power
************************************/
void RF_PowerOn(void);
void RF_Lowpower_Set(void);
void RF_Scan_Fun(void *priv);












































#endif
