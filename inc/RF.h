#ifndef _RF_H
#define _RF_H
#include "stm32f0xx.h"
//#include"aeplatform.h"
//��MF522ͨѶʱ���صĴ������
#define MI_OK                          0
#define MI_NOTAGERR                    1
#define MI_ERR                         2


//RFID 1356һЩ����״̬
#define RFID1356_READ_CARD_OK                       0x02
#define RFID1356_DISABLE_CARD_RX                    0x01
#define RFID1356_ENABLE_CARD_RX                     0x00

//Mifare_One��Ƭ������
#define PICC_REQIDL           0x26               //Ѱ��������δ��������״̬
#define PICC_REQALL           0x52               //Ѱ��������ȫ����
#define PICC_ANTICOLL1        0x93               //����ײ
#define PICC_ANTICOLL2        0x95               //����ײ
#define PICC_AUTHENT1A        0x60               //��֤A��Կ
#define PICC_AUTHENT1B        0x61               //��֤B��Կ
#define PICC_READ             0x30               //����
#define PICC_WRITE            0xA0               //д��
#define PICC_DECREMENT        0xC0               //�ۿ�
#define PICC_INCREMENT        0xC1               //��ֵ
#define PICC_RESTORE          0xC2               //�������ݵ�������
#define PICC_TRANSFER         0xB0               //���滺����������
#define PICC_HALT             0x50               //����
//MF522�Ĵ�������
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
//MF522������
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //ȡ����ǰ����
#define PCD_AUTHENT           0x0E               //��֤��Կ
#define PCD_RECEIVE           0x08               //��������
#define PCD_TRANSMIT          0x04               //��������
#define PCD_TRANSCEIVE        0x0C               //���Ͳ���������
#define PCD_RESETPHASE        0x0F               //��λ
#define PCD_CALCCRC           0x03               //CRC����

//MF522 FIFO���ȶ���
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

#define MAXRLEN               18
//ˢ�����ܲ��ֺ궨��
#define ENCRYPT_ALL_BLOCK  0//���������������
#ifndef ENCRYPT_ALL_BLOCK  //���û���������������,����ָ����Ҫ���ܵĵ����������0~15
    #define ENCRYPT_BLOCK_NUM           10//��������������������,Ҳ���������һ������,��ô�Կ�������֤,���еĿ�������ʶ��
#endif


#define RF_VOL_AVERAGE_MIN_THRESHOLD    50
#define RF_VOL_WAKEUP_PERCENT_MIN    10/100
#define RF_VOL_WAKEUP_PERCENT_MAX    90/100

/*****************************
��������:RF_Init
����:
��ʼ��spi IO��RC522����
����:
��
����ֵ:��
******************************/

void RF_Init(void);


/********************************
���ܣ���RC522�Ĵ���λ
������reg:�Ĵ�����ַ
      mask:��λֵ�������õ���λ����Ϊ0x80
����ֵ:��
*********************************/
void RF_SetBitMask(uint8_t reg,uint8_t mask)  ;

/********************************
���ܣ���RC522�Ĵ���λ
������reg:�Ĵ�����ַ
      mask:��λֵ
����ֵ:��
*********************************/
void RF_ClearBitMask(uint8_t reg,uint8_t mask) ;

/*************************
����:����RC522����
��������
����ֵ:��
��ע:ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
*************************/
void RF_PcdAntennaOn(void);

/*************************
����:�ر�RC522����
��������
����ֵ:��
��ע:ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
*************************/
void RF_PcdAntennaOff(void);

/***********************************
��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
����˵����Command[IN]:RC522������
          pInData[IN]:ͨ��RC522���͵���Ƭ������
          InLenByte[IN]:�������ݵ��ֽڳ���
          pOutData[OUT]:���յ��Ŀ�Ƭ��������
          *pOutLenBit[OUT]:�������ݵ�λ����
����ֵ:    MI_OK     �ɹ�����
           MI_NOTAGERR    ͨ�Ų��ɹ�
           MI_ERR    ���󷵻�
**************************************/
uint8_t RF_PcdComMF522(  uint8_t Command, 
                                   uint8_t *pInData, 
                                   uint8_t InLenByte,
                                   uint8_t *pOutData, 
                                   uint8_t  *pOutLenBit);

/**********************************
��    �ܣ�Ѱ��
����˵��:   req_code[IN]:Ѱ����ʽ
                    0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
                    0x26 = Ѱδ��������״̬�Ŀ�
            pTagType[OUT]����Ƭ���ʹ���
                    0x4400 = Mifare_UltraLight
                    0x0400 = Mifare_One(S50)
                    0x0200 = Mifare_One(S70)
                    0x0800 = Mifare_Pro(X)
                    0x4403 = Mifare_DESFire
����ֵ:    MI_OK     �ɹ�����
           MI_NOTAGERR    ͨ�Ų��ɹ�
           MI_ERR    ���󷵻�
            
************************************/
uint8_t RF_PcdRequest(uint8_t req_code,uint8_t *pTagType);


/******************************
��    �ܣ�����ײ
����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*******************************/  
uint8_t RF_PcdAnticoll(uint8_t *pSnr);

/************************************
˵  ��:��MF522����CRC16����
��  ��:     pIndata ��������ݻ�����
            len ��������ݳ���
            pOutData ��������ݻ�����
����ֵ:��
*************************************/
void RF_CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData);

/************************************
˵  ��:ѡ����Ƭ
��  ��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/
uint8_t RF_PcdSelect(uint8_t *pSnr);

/************************************
˵  ��:���Ƭ��������״̬
��  ��:��
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/
uint8_t RF_PcdHalt(void);

/************************************
˵  ��:��ȡ��
��  ��:
        Card_type[out] ��������,
                        0:m1 s50��
                        1:m1 s70��
        Card_num[out]  ����
                        (4�ֽڿ���)
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/
uint8_t RF_GetCard(uint8_t *Card_type,uint8_t *Card_num);
/************************************
˵  ��: ��֤��Ƭ����
��  ��: 
        auth_mode[IN]: ������֤ģʽ
                 0x60 ��֤A��Կ
                 0x61 ��֤B��Կ
        addr:���ַ
        pKey[IN]:����
        pSnr[IN]:��Ƭ���кţ�4�ֽ�
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/

uint8_t RF_PcdAuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *Psnr);

/************************************
˵  ��: ��ȡM1��һ�������
��  ��: 
        addr:���ַ
        pData[OUT]:���������ݣ�16�ֽ�
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/

uint8_t RF_PcdRead(uint8_t addr,uint8_t *pData);

/************************************
˵  ��: ��M1��ĳ������3��ַ������д������
��  ��: 
        
        auth_mode[IN]: ������֤ģʽ
                 0x60 ��֤A��ԿPICC_AUTHENT1A
                 0x61 ��֤B��ԿPICC_AUTHENT1B
        addr:���ַ(�ѿ�Ĭ���������0��ʼ��ţ�����S50��������0~63��)
            addr=3+n*4;
        pucKey[IN]::ҪҪ��֤������
        pucData[OUT]:����д������롣
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/
uint8_t RF_RelaseBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData);

/************************************
˵  ��:��M1����ĳ���ַ�����¶�������
��  ��:         
        auth_mode[IN]: ������֤ģʽ
                 0x60 ��֤A��ԿPICC_AUTHENT1A
                 0x61 ��֤B��ԿPICC_AUTHENT1B
        addr:���ַ(�ѿ�Ĭ���������0��ʼ��ţ�����S50��������0~63��)
             addr=n;
        pucKey[IN]: Ҫ��֤������,(���������Ϳ�������һ�£����У�鲻�ɹ������ܶ�ȡ����)
        pucData[OUT]:����Ŀ��ֽ����ݣ���16�ֽڡ�
����ֵ: MI_OK     �ɹ�����
        MI_NOTAGERR    ͨ�Ų��ɹ�
        MI_ERR    ���󷵻�
*************************************/
uint8_t RF_ReadBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData);

/***********************************
˵����Tx1��Tx2�Ͻ��ᷢ��13.56MHZ���ز�����
************************************/
void RF_TurnON_TX_Driver_Data(void);

/***********************************
RF Reset and low power
************************************/
void RF_PowerOn(void);
void RF_Lowpower_Set(void);
void RF_Scan_Fun(void *priv);












































#endif
