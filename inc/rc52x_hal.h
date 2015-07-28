#ifndef _RC52X_HAL_H_
#define _RC52X_HAL_H_
/////////////////////////////////////////////////////////////////////

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

#define  RF_RESET_PORT  GPIOB
#define  RF_RESET_PIN   GPIO_Pin_7

uint8_t RF1356_MasterReadData(uint8_t addr);
unsigned char RF1356_MasterWriteData(unsigned char addr,unsigned char wrdata);
uint8_t RF1356_RC523Init(void);
void RF1356_GetCard(void);


#endif

