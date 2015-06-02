#ifndef __74h595_h_
#define __74h595_h_
#include "stm32f0xx.h"
#include "stdio.h"
#include "stdbool.h"

//led and seg8_led share this struct
#define SER_LED_INTERFACE 0x01
#define SER_DOT_INTERFACE 0x02
/* Modes */
#define HAL_LED_MODE_OFF           0x00    //no operation
#define HAL_LED_MODE_ON      				0x01   // display on œ‘ æ
#define HAL_LED_MODE_BLINK   				0x02   //…¡À∏
#define HAL_LED_MODE_FLASH   				0x04   //“ª÷±…¡À∏
#define HAL_LED_MODE_BLINK_STOP     0x08     // Õ£÷π…¡À∏
#define HAL_LED_MODE_ON_CONTINUE    0x10   //continue on times 

	
/* SEG8 LED control structure */
typedef struct {
	uint8_t SleepActive;
  uint8_t mode;       /* Operation mode */
  uint32_t left;       /* …¡À∏¥Œ ˝ */
  uint32_t ontime;      /* LED on time */
  uint32_t offtime;      /*  OFF(msec) */
  uint32_t next;      /* Time for next change */
	uint16_t DisplayCode;      /* Seg8 digital led display code */
} HalLedControl_t;


void HC595_init(uint8_t number);
bool HC595_ShiftOut(uint8_t num, uint8_t value);
bool HC595_ShiftOut16(uint8_t num,uint16_t code);
bool HC595_ShiftOut24(uint8_t num,uint16_t code);
bool HC595_Updata(uint8_t num, uint8_t value);
void HC595_Power_ON(void);
void HC595_Power_OFF(void);
bool HC595_Latch(uint8_t num, uint8_t state);


#endif




