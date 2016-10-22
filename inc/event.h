#ifndef __EVENT_H_
#define __EVENT_H_
#include "stm32f0xx.h"
#include "index.h"

#define LOCK_EVENT_RING_SIZE  64

typedef enum
{
	EVENT_NONE = 0x00,
	BUTTON_KEY_EVENT,
	TOUCH_KEY_EVENT,
	RFID_CARD_EVENT,
	FINGER_EVENT,
	AU_EVENT
}Lock_EventTypeTypeDef;

typedef union
{
	uint8_t KeyValude;
	
	uint8_t Buff[20];
}EventDataTypeDef;

typedef struct
{
	Lock_EventTypeTypeDef event;
	uint8_t len;
	EventDataTypeDef data;
}Hal_EventTypedef;

Hal_EventTypedef GetEvent(void);
void PutEvent(Hal_EventTypedef e);
void ClearAllEvent(void);

#endif
