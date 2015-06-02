#ifndef __BUTTON_KEY_H_
#define __BUTTON_KEY_H_

#include "stm32f0xx.h"
#include "Link_list.h"

#define 	KEY_CANCEL_SHORT	 1
#define 	KEY_DEL_SHORT 		 2
#define 	KEY_OK_SHORT  		 4
#define 	KEY_INIT_SHORT 		 8
#define 	KEY_ADD_SHORT   	0x10

#define 	KEY_CANCEL_LONG	 KEY_CANCEL_SHORT |0x80
#define 	KEY_DEL_LONG 		 KEY_DEL_SHORT |0x80
#define 	KEY_OK_LONG  		 KEY_OK_SHORT |0x80
#define 	KEY_INIT_LONG 	 KEY_INIT_SHORT |0x80
#define 	KEY_ADD_LONG   	 KEY_ADD_SHORT |0x80

void Button_Key_Init(void);  //TODO 以后要改成中断方式

#endif

