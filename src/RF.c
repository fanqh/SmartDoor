#include "RF.h"
#include"RF_Hardware.h"
#include "string.h"

#include"delay.h"
#include "Link_list.h"
#include "event.h"
#include "process_event.h"
#include "pwm.h"
#include "adc.h"
//#include "rf_vol_judge.h"
//#include"Bootapi.h"

struct node_struct_t RF_Scan_Node;

//RFID 1356.c
uint8_t g_cGetCardStatus=1;
uint8_t g_cInitStatusEnRdCardFlag=0;//ÔÚÉÏµç±È¶Ô×´Ì¬ÏÂ£¬¸Ã±êÖ¾£¬±êÖ¾ÊÇ·ñÔÊÐí¶Á¿¨
uint8_t g_cCardTestingStatus=0x00;//¿¨Ä£¿é´¦ÓÚ±»²âÊÔ×´Ì¬
extern uint8_t is_Err_Warm_Flag;

void RF_Scan_Fun(void *priv);

unsigned char g_cCardPwd[16][6] = 
{
    {0x02,0x32,0x56,0x12,0x34,0x56},
    {0x12,0xAA,0x56,0x12,0x34,0x56},
    {0x12,0x34,0x56,0x12,0x39,0x96},
    {0x12,0x34,0x55,0x32,0x34,0x56},
    {0x1E,0xE4,0x56,0x18,0x53,0x46},
    {0x12,0x34,0xA6,0x12,0x3B,0x56},
    {0x12,0x30,0x61,0x23,0xF5,0x6F},
    {0x99,0x34,0x56,0x12,0x34,0x56},
    {0x12,0xB5,0x61,0x23,0x4A,0x6F},
    {0x12,0x37,0x86,0x12,0x34,0xC6},
    {0x1A,0x34,0xCC,0x12,0x34,0x56},
    {0x1A,0x34,0x56,0x1F,0xF4,0x56},
    {0x12,0xE9,0x56,0x12,0x39,0x59},
    {0x15,0x5E,0x56,0x1C,0x45,0x99},
    {0xEE,0x34,0x56,0x05,0x34,0x56},
    {0x12,0xB4,0x56,0x53,0x34,0x54}
};
void RF_SetBitMask(uint8_t reg,uint8_t mask)  
{
    uint8_t tmp = 0x00;
    tmp = RF_MasterReadData(reg);
    RF_MasterWriteData(reg,tmp | mask);  // set bit mask
}

void RF_ClearBitMask(uint8_t reg,uint8_t mask)  
{
    uint8_t tmp = 0x00;
    tmp = RF_MasterReadData(reg);
    RF_MasterWriteData(reg, tmp & (~mask));  // clear bit mask
} 

void RF_PcdAntennaOn(void)
{
    unsigned char i;
    i = RF_MasterReadData(TX_CONTROL_REG);
    if (!(i & 0x03))
    {
        RF_SetBitMask(TX_CONTROL_REG, 0x03);
    }
}

void RF_PcdAntennaOff(void)
{
    RF_ClearBitMask(TX_CONTROL_REG, 0x03);
}
/***********************************
¹¦    ÄÜ£ºÍ¨¹ýRC522ºÍISO14443¿¨Í¨Ñ¶
²ÎÊýËµÃ÷£ºCommand[IN]:RC522ÃüÁî×Ö
          pInData[IN]:Í¨¹ýRC522·¢ËÍµ½¿¨Æ¬µÄÊý¾Ý
          InLenByte[IN]:·¢ËÍÊý¾ÝµÄ×Ö½Ú³¤¶È
          pOutData[OUT]:½ÓÊÕµ½µÄ¿¨Æ¬·µ»ØÊý¾Ý
          *pOutLenBit[OUT]:·µ»ØÊý¾ÝµÄÎ»³¤¶È
·µ»ØÖµ:    MI_OK     ³É¹¦·µ»Ø
           MI_NOTAGERR    Í¨ÐÅ²»³É¹¦
           MI_ERR    ´íÎó·µ»Ø
**************************************/
uint8_t RF_PcdComMF522(  uint8_t Command, 
                                   uint8_t *pInData, 
                                   uint8_t InLenByte,
                                   uint8_t *pOutData, 
                                   uint8_t  *pOutLenBit)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;
    
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
    RF_MasterWriteData(COM_IEN_REG,irqEn|0x80);
    RF_ClearBitMask(COM_IRQ_REG,0x80);
    RF_MasterWriteData(COMMAND_REG,PCD_IDLE);
    RF_SetBitMask(FIFO_LEVEL_REG,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
        RF_MasterWriteData(FIFO_DATA_REG, pInData[i]);    
    }
    RF_MasterWriteData(COMMAND_REG, Command);
   
    if (Command == PCD_TRANSCEIVE)
    {    
        RF_SetBitMask(BIT_FRAMING_REG,0x80);  
    }
    //¸ù¾ÝÊ±ÖÓÆµÂÊµ÷Õû£¬²Ù×÷M1¿¨×î´óµÈ´ýÊ±¼ä25ms
   // i=100;
   // i = 600;
    i = 2000;
    do 
    {
         n = RF_MasterReadData(COM_IRQ_REG);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    RF_ClearBitMask(BIT_FRAMING_REG,0x80);
	      
    if (i!=0)
    {    
         if(!(RF_MasterReadData(ERROR_REG)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   
                 status = MI_NOTAGERR;   
             }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = RF_MasterReadData(FIFO_LEVEL_REG);
              	lastBits = RF_MasterReadData(CONTROL_REG) & 0x07;
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
                    pOutData[i] = RF_MasterReadData(FIFO_DATA_REG)    ;
                }
            }
         }
         else
         {   
             status = MI_ERR;   
         }
   }
   RF_SetBitMask(CONTROL_REG,0x80);           // stop timer now
   RF_MasterWriteData(COMMAND_REG,PCD_IDLE); 
   return status;
}
/**********************************
¹¦    ÄÜ£ºÑ°¿¨
²ÎÊýËµÃ÷:   req_code[IN]:Ñ°¿¨·½Ê½
                    0x52 = Ñ°¸ÐÓ¦ÇøÄÚËùÓÐ·ûºÏ14443A±ê×¼µÄ¿¨
                    0x26 = Ñ°Î´½øÈëÐÝÃß×´Ì¬µÄ¿¨
            pTagType[OUT]£º¿¨Æ¬ÀàÐÍ´úÂë
                    0x4400 = Mifare_UltraLight
                    0x0400 = Mifare_One(S50)
                    0x0200 = Mifare_One(S70)
                    0x0800 = Mifare_Pro(X)
                    0x4403 = Mifare_DESFire
·µ»ØÖµ:    MI_OK     ³É¹¦·µ»Ø
           MI_NOTAGERR    Í¨ÐÅ²»³É¹¦
           MI_ERR    ´íÎó·µ»Ø
            
************************************/
uint8_t RF_PcdRequest(uint8_t req_code,uint8_t *pTagType)
{
   uint8_t status;  
   uint8_t  unLen;
   uint8_t ucComMF522Buf[MAXRLEN]; 
   
   RF_ClearBitMask(STATUS2_REG,0x08);
   RF_MasterWriteData(BIT_FRAMING_REG,0x07);
   RF_SetBitMask(TX_CONTROL_REG,0x03);
 
   ucComMF522Buf[0] = req_code;

   status = RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

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


/******************************
¹¦    ÄÜ£º·À³å×²
²ÎÊýËµÃ÷: pSnr[OUT]:¿¨Æ¬ÐòÁÐºÅ£¬4×Ö½Ú
·µ»ØÖµ: MI_OK     ³É¹¦·µ»Ø
        MI_NOTAGERR    Í¨ÐÅ²»³É¹¦
        MI_ERR    ´íÎó·µ»Ø
*******************************/  
uint8_t RF_PcdAnticoll(uint8_t *pSnr)
{
    uint8_t status;
    uint8_t i,snr_check=0;
    uint8_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 
    

    RF_ClearBitMask(STATUS2_REG,0x08);
    RF_MasterWriteData(BIT_FRAMING_REG,0x00);
    RF_ClearBitMask(COLL_REG,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

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
    RF_SetBitMask(COLL_REG,0x80);
    return status;
}

/************************************
Ëµ  Ã÷:ÓÃMF522¼ÆËãCRC16º¯Êý
²Î  Êý:     pIndata ÊäÈëµÄÊý¾Ý»º³åÇø
            len ÊäÈëµÄÊý¾Ý³¤¶È
            pOutData Êä³öµÄÊý¾Ý»º³åÇø
·µ»ØÖµ:ÎÞ
*************************************/
void RF_CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData)
{
    uint8_t i,n;
    RF_ClearBitMask(DIV_IRQ_REG,0x04);
    RF_MasterWriteData(COMMAND_REG,PCD_IDLE);
    RF_SetBitMask(WATER_LEVEL_REG,0x80);
    for (i=0; i<len; i++)
    {   
        RF_MasterWriteData(FIFO_DATA_REG, *(pIndata+i));   
    }
    RF_MasterWriteData(COMMAND_REG, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = RF_MasterReadData(DIV_IRQ_REG);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = RF_MasterReadData(CRC_RESULT_REG_L);
    pOutData[1] = RF_MasterReadData(CRC_RESULT_REG_M);
}

/************************************
Ëµ  Ã÷:Ñ¡¶¨¿¨Æ¬
²Î  Êý: pSnr[IN]:¿¨Æ¬ÐòÁÐºÅ£¬4×Ö½Ú
·µ»ØÖµ: MI_OK     ³É¹¦·µ»Ø
        MI_NOTAGERR    Í¨ÐÅ²»³É¹¦
        MI_ERR    ´íÎó·µ»Ø
*************************************/
uint8_t RF1356_PcdSelect(uint8_t *pSnr)
{
    uint8_t status;
    uint8_t i;
    uint8_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    RF_CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    RF_ClearBitMask(STATUS2_REG,0x08);

    status = RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
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
/************************************
Ëµ  Ã÷:ÃüÁî¿¨Æ¬½øÈëÐÝÃß×´Ì¬
²Î  Êý:ÎÞ
·µ»ØÖµ: MI_OK     ³É¹¦·µ»Ø
*************************************/
uint8_t RF_PcdHalt(void)
{
    uint8_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    RF_CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

uint8_t RF_GetCard(uint8_t *Card_type,uint8_t *Card_num)
{
    uint8_t temp;
    uint8_t ucRfidRxBuff[20];
    uint16_t uwCardCode;
    memset(ucRfidRxBuff,0xff,20);
    temp = RF_PcdRequest(PICC_REQIDL, ucRfidRxBuff);
    if(temp != MI_OK)
        return MI_ERR;
    uwCardCode=ucRfidRxBuff[0];
    uwCardCode=(uwCardCode<<8)|(ucRfidRxBuff[1]);
    if(uwCardCode==0x0400)
        Card_type[0]=0x00; //M1 s50¿¨
    else if(uwCardCode==0x0200)
        Card_type[0]=0x01;//M1 s70¿¨
    else
        Card_type[0]=0xff;
    
    temp = RF_PcdAnticoll(ucRfidRxBuff);
    if(temp != MI_OK)
        return MI_ERR;
    temp = RF1356_PcdSelect(ucRfidRxBuff);
    if(temp != MI_OK)
        return MI_ERR;
    RF_PcdHalt();
    memcpy(Card_num,ucRfidRxBuff,4);
    return MI_OK;
}

uint8_t RF_PcdAuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
{
    uint8_t status;
    uint8_t unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN];
    
    ucComMF522Buf[0]=auth_mode;//ÑéÖ¤AÃØÔ¿
    ucComMF522Buf[1]=addr;//addr¿éµØÖ·
    for(i=0;i<6;i++)
    {
        ucComMF522Buf[i+2]=*(pKey+i);
    }
    for(i=0;i<6;i++)
    {
        ucComMF522Buf[i+8]=*(pSnr+i);
    }
    status=RF_PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if((status!=MI_OK)||(!(RF_MasterReadData(STATUS2_REG)&0x08)))
    {
        status=MI_ERR;
    }
    return status;
}

uint8_t RF_PcdRead(uint8_t addr,uint8_t *pData)
{
    uint8_t status=0;
    uint8_t unLen;
    uint8_t i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0]=PICC_READ;
    ucComMF522Buf[1]=addr;
    
    RF_CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
    RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if((status==MI_OK)&&(unLen==0x90))
    {
        for(i=0;i<16;i++)
        {
            *(pData+i)=ucComMF522Buf[i];
        }    
    }
    else
    {
        status=MI_ERR;
    }
    return status;
}
                 
unsigned char RF_PcdWrite(uint8_t addr,uint8_t *pData)
{
    uint8_t status;
    uint8_t  ucLen;
    uint8_t i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    RF_CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&ucLen);

    if ((status != MI_OK) || (ucLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   
        status = MI_ERR;   
    }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    
            ucComMF522Buf[i] = *(pData+i);   
        }
        RF_CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = RF_PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&ucLen);
        if ((status != MI_OK) || (ucLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   
            status = MI_ERR;   
        }
    }
    
    return status;
}


uint8_t RF_RelaseBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData)
{
    uint8_t ucResult;
    uint8_t ucRfidRxBuff[20];
    uint8_t pwd[6] ;//= {0xff,0xff,0xff,0xff,0xff,0xff};
    uint8_t block_buff[16] ;//= {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};//88 78 77 00
    uint8_t i;
    if((addr-3)%4!=0)
        return MI_ERR;
    for(i=0;i<6;i++)
    {
        pwd[i]=pucKey[i];

    }
    for(i=0;i<16;i++)
    {
        block_buff[i]=pucData[i];
    }
    ucResult = RF_PcdRequest(PICC_REQALL, ucRfidRxBuff);//Ñ°¿¨
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("a", 1);
    ucResult = RF_PcdAnticoll(ucRfidRxBuff);//·À³åÍ»
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("b", 1);
    ucResult = RF1356_PcdSelect(ucRfidRxBuff);//Ñ¡¿¨  
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("c", 1);
    ucResult= RF_PcdAuthState(auth_mode,addr,pwd,ucRfidRxBuff);//Ð£ÑéÃØÔ¿ 
    if(ucResult != MI_OK)
        return MI_ERR; 
   // Boot_UsartSend("d", 1);
    ucResult = RF_PcdWrite(addr,block_buff);//Ð´ÈëÐÂµÄÃÜÂë
    if(ucResult != MI_OK)
        return MI_ERR; 
    //Boot_UsartSend("e", 1);
    RF_PcdHalt();
    return MI_OK;
}

uint8_t RF_ReadBlock(uint8_t auth_mode,uint8_t addr,uint8_t *pucKey,uint8_t *pucData)
{
    uint8_t ucResult;
    uint8_t ucRfidRxBuff[20];
    uint8_t pwd[6] ;
    uint8_t block_buff[16] ;
    uint8_t i;
    for(i=0;i<6;i++)
    {
        pwd[i]=pucKey[i];
    }
    
    ucResult = RF_PcdRequest(PICC_REQALL, ucRfidRxBuff);//Ñ°¿¨
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("a", 1);
    ucResult = RF_PcdAnticoll(ucRfidRxBuff);//·À³åÍ»
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("b", 1);
    ucResult = RF1356_PcdSelect(ucRfidRxBuff);//Ñ¡¿¨  
    if(ucResult != MI_OK)
        return MI_ERR; 
    //Boot_UsartSend("c", 1);
    ucResult= RF_PcdAuthState(auth_mode,addr,pwd,ucRfidRxBuff);//Ð£ÑéÃØÔ¿ 
    if(ucResult != MI_OK)
        return MI_ERR;
    //Boot_UsartSend("d", 1);
    ucResult = RF_PcdRead(addr,block_buff);//¶Á³ö¿éµÄ16×Ö½ÚÊý¾Ý¡£
    if(ucResult != MI_OK)
        return MI_ERR; 
    //Boot_UsartSend("e", 1);
    memcpy(pucData,block_buff,16);
    RF_PcdHalt();  
    return MI_OK;
}
void RF_Init(void)
{
//    delay_us(150);//RFµçÔ´¿¨¿ªºóÖÁÉÙÑÓ³Ù120us,²Å¿ÉÒÔ½øÐÐRFµÄÅäÖÃ£¬·ñÔòÅäÖÃ²»³É¹¦£
                    //¾ßÌåÊ²Ã´Ô­Òò£¬ÐèÒªÁ¿µçÔ´²¨ÐÎÓëÐÅºÅÖ®¼äµÄ¹ØÁª¡£??????
//    RF_Spi_Config();
    RF_Reset_High();//RST = 1
    delay_us(1);
    RF_Reset_Low();//RST = 0
    delay_us(1);
    RF_Reset_High();//RST = 1
    delay_us(1);
    RF_MasterWriteData(COMMAND_REG,PCD_RESETPHASE);//¸´Î»ÃüÁî
    delay_us(1);
    
    RF_MasterWriteData(MODE_REG,0x3D);            //ºÍMifare¿¨Í¨Ñ¶£¬CRC³õÊ¼Öµ0x6363
    RF_MasterWriteData(TRELOAD_REG_L,30);           
    RF_MasterWriteData(TRELOAD_REG_H,0);
    RF_MasterWriteData(TMODE_REG,0x8D);
    RF_MasterWriteData(TPRESCALER_REG,0x3E);
    RF_MasterWriteData(TX_AUTO_REG,0x40);  
    
    RF_PcdAntennaOff();//¹Ø±ÕÌìÏß
	
		lklt_insert(&RF_Scan_Node, RF_Scan_Fun, NULL, 10*TRAV_INTERVAL);  //
    //RF_PcdAntennaOn();//¿ªÆôÌìÏß

    /*
   // ISO ÀàÐÍ A
    RF_ClearBitMask(STATUS2_REG,0x08);
    RF_MasterWriteData(MODE_REG,0x3D);//3F    
    RF_MasterWriteData(RX_SEL_REG,0x86);//84    
    RF_MasterWriteData(RF_CFG_REG,0x7F);   //4F    
    RF_MasterWriteData(TRELOAD_REG_L,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
    RF_MasterWriteData(TRELOAD_REG_H,0);
    RF_MasterWriteData(TMODE_REG,0x8D);//
    RF_MasterWriteData(TPRESCALER_REG,0x3E);
    delay_ms(1);
    RF_PcdAntennaOn();*/
}

void RF_TurnON_TX_Driver_Data(void)
{
		RF_SetBitMask(TX_CONTROL_REG,0x03);	
}
void RF_PowerOn(void)
{
//    RF_Reset_Low();//RST = 0
//    delay_us(1);
    RF_Reset_High();//RST = 1
 //   delay_us(1);	
//	RF_MasterWriteData(COMMAND_REG,0x30);//power down
}

void RF_Lowpower_Set(void)
{
	
	RF_PcdAntennaOff();//¹Ø±ÕÌìÏß	
	RF_SetBitMask(COMMAND_REG,0x30);
//	RF_MasterWriteData(COMMAND_REG,0x30);//power down
//	while(!(RF_MasterReadData(COMMAND_REG)&0x30));
//	printf("rf turn off\r\n");
}
extern Hal_EventTypedef gEventOne;
void RF_Scan_Fun(void *priv)
{
		uint8_t cardType =0;
		uint8_t cardNum[5];
		Hal_EventTypedef evt;
//		uint8_t i;
//		uint32_t vol, average;
	
	    if(is_Err_Warm_Flag==1)
			return;
		switch(lock_operate.lock_state)
		{
			case WAIT_PASSWORD_ONE:
			case LOCK_READY:	
			case WAIT_AUTHENTIC:
			case DELETE_USER_BY_FP:
			case DELETE_ADMIN_BY_FP:
//			case LOCK_OPEN_NORMAL:
//			ADC1_CH_DMA_Config();
//			vol =  Get_RF_Voltage();
//			average = GetAverageVol(FLASH_PAGE_SIZE*FLASH_VOL_PAGE);

//				printf("vol=%dmV\r\n", vol);
#if 1
//			if((vol>(average*RF_VOL_WAKEUP_PERCENT_MIN))&&(vol<RF_VOL_WAKEUP_PERCENT_MAX)&&(average!=0xffffffff))
			{
				
				if(RF_GetCard(&cardType,cardNum)==MI_OK)
				{
					char null[4]= {0,0,0,0};
					cardNum[4]='\0';
					if(strcmp(cardNum, null)!=0)
					{
							printf("scan ok\r\n");
							evt.event = RFID_CARD_EVENT;
							memcpy(evt.data.Buff, cardNum, sizeof(cardNum));
							PutEvent(evt);
							//Hal_Beep_Blink (1, 80, 30);//Ã»±ØÒª´ø×Å£¬£¬ÒòÎªprocess_event()ÖÐ»áÓÐÌáÊ¾Òô
//							printf("cardNum: \r\n");
//							for(i=0;i<4;i++)
//							{
//								printf("%X",cardNum[i]);
//							}
//							printf("\r\n");
					}
				}
			}
			break;
			case WATI_PASSWORD_TWO:
				if(gEventOne.event != RFID_CARD_EVENT)
					break;
				if(gEventOne.event == RFID_CARD_EVENT)
				{
					if(RF_GetCard(&cardType,cardNum)==MI_OK)
					{
						char null[4]= {0,0,0,0};
						cardNum[4]='\0';
						if(strcmp(cardNum, null)!=0)
						{
							evt.event = RFID_CARD_EVENT;
							memcpy(evt.data.Buff, cardNum, sizeof(cardNum));
							PutEvent(evt);
							Hal_Beep_Blink (1, 80, 30);
						}
					}
				}
				break;
#endif
			default:
				break;
		}
}
  





























































