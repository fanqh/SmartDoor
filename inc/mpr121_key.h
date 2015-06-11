#ifndef _MPR121_KEY_H_
#define _MPR121_KEY_H_
#include "stdint.h"

#define MAX_KEY_NUM             12
#define LONG_KEY_MASK           0x80
#define TOUCH_SHORT_TIME        3
#define TOUCH_LONG_TIME         40

extern uint8_t touch_key_buf[20];
extern fifo_struct touch_key_fifo;

void clr_touch_key_buf(void);
int16_t acquire_touch_key(void);
int16_t get_touch_key(uint8_t *pucKey,uint32_t time_out);
void mpr121_exti_interrupt(void);
void mpr121_init_config(void);
void mpr121_backlight_open(void);
void mpr121_backlight_close(void);
int16_t mpr121_reopen(void);
int16_t mpr121_close(void);
int16_t mpr121_disable(void);
int16_t mpr121_enter_standby(void);
void touch_key_hadware_init(void);
uint16_t is_touch_wkup(void);
unsigned char Get_fifo_size(fifo_struct *fifo_struct);


void touch_key_scan(void *priv);   
#endif

