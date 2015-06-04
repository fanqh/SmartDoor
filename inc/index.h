#ifndef __INDEX_H_
#define __INDEX_H_
#include "stm32f0xx.h"

#define INDEX_ADDR_START  					0X0800EC00
#define USER_ID_PAGE0_ADDR_START  	0X0800F000
#define USER_ID_PAGE1_ADDR_START  	0X0800F400
#define USER_ID_PAGE2_ADDR_START  	0X0800F800
#define ADMIN_ID_PAGE4_ADDR_START   0X0800FC00

typedef struct{
	uint16_t flag;
	uint8_t id;
	uint8_t type;
	uint16_t len;
	uint8_t password[20];
	uint8_t reserved[7];
}id_infor_t;


typedef struct{
	uint16_t x;
	uint16_t y;
}index_mapping_t;

typedef enum{
 
	SAVE_OK = 0,
	SAVE_FAIL
}FLASH_STATUS;

typedef enum{
	NORMAL=0,
	SECURITY
}work_mode_t;


typedef struct{
	uint16_t flag;
	work_mode_t work_mode;
	index_mapping_t index_map;
}lock_infor_t;

extern lock_infor_t lock_infor;

FLASH_STATUS Flash_Write(uint32_t addr, uint32_t *src, uint16_t len);
void Index_Read(void);
void Index_Init(void);
FLASH_STATUS Index_Save(void);
uint8_t Get_id_Number(void);
int8_t Get_User_id_Number(void);
uint8_t Get_Admin_id_Number(void);
uint8_t Get_id_Number(void);
int Find_Next_ID(uint8_t id);
int Find_Next_Null_ID(uint8_t id);

#endif
