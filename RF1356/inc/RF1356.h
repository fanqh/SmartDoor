#ifndef __RF1356_H_
#define __RF1356_H_

#include "main.h"
#include "global.h"
#include "lpcd_api.h"

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_spi.h"

/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define MI_OK                          0
#define MI_NOTAGERR                    1
#define MI_ERR                         2


#define CARD_IRQ_PIN                              GPIO_Pin_7
#define CARD_IRQ_PORT	                      GPIOF

//spi interface and rst pin maco

#define RFID_CTRL_PORT                          GPIOB 
#define RFID_MOSI_PIN                        	GPIO_Pin_5
#define RFID_CLK_PIN                        	GPIO_Pin_3
#define RFID_MISO_PIN                        	GPIO_Pin_4
#define RFID_CS_PIN                        		GPIO_Pin_6
#define RFID_RESET_PIN                     		GPIO_Pin_7 

#define  RF1356_SET_RESET_HIGH()    GPIO_SetBits(RFID_CTRL_PORT, RFID_RESET_PIN) /*拉高 复位*/
#define  RF1356_SET_RESET_LOW()	    GPIO_ResetBits(RFID_CTRL_PORT, RFID_RESET_PIN) 

void RF1356_MasterWriteData(unsigned char addr,unsigned char wrdata);
unsigned char RF1356_MasterReadData(unsigned char addr);
unsigned char RF1356_MasterReadData2(unsigned char addr,unsigned char *p);
void RF1356_MasterInit(void);

unsigned char RF1356_RC523Init(void);
int16_t RF1356_GetCard(uint8_t *ptr);
int16_t RTC_WakeUp_GetCard(uint8_t *ptr);

void Rf1356_InitGetCard(void);
void RF1356_SleepCheckCard(void);
void card_irq_init(void);
uint8_t card_irq_status(void);
							  
#endif

