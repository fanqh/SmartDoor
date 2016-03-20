#ifndef _MPR121_KEY_H_
#define _MPR121_KEY_H_
#include "stdint.h"
#include "fifo.h"

#define MAX_KEY_NUM             12
#define LONG_KEY_MASK           0x80
#define TOUCH_SHORT_TIME        2    //2ms
#define TOUCH_LONG_TIME         5*1000*2    //300ms

#define TOUCH_KEY_PSWD_MIN_LEN			8
#define TOUCH_KEY_PSWD_LEN   			8
#define TOUCH_KEY_PSWD_MAX_LEN    		17
#define RFID_CARD_NUM_LEN        		 4
#define FINGER_INDEX_LEN				2

extern uint8_t touch_key_buf[TOUCH_KEY_PSWD_MAX_LEN+1];
extern fifo_struct touch_key_fifo;


void mpr121_init_config(void);
int16_t mpr121_enter_standby(void);

void mpr121_IRQ_Pin_Config(void);
uint8_t mpr121_get_irq_status(void);
uint8_t mpr121_get_irq_debounce(void);


void touch_key_scan(void *priv);   
#endif

