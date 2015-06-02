#ifndef _RC52X_HAL_H_
#define _RC52X_HAL_H_
/////////////////////////////////////////////////////////////////////

#define  RF_RESET_PORT  GPIOB
#define  RF_RESET_PIN   GPIO_Pin_7

uint8_t RF1356_MasterReadData(uint8_t addr);
unsigned char RF1356_MasterWriteData(unsigned char addr,unsigned char wrdata);
uint8_t RF1356_RC523Init(void);
void RF1356_GetCard(void);


#endif

