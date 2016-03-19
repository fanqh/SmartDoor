#ifndef __INDEX_H_
#define __INDEX_H_
#include "stm32f0xx.h"

#define INDEX_ADDR_START  					0X0800EC00
#define USER_ID_PAGE0_ADDR_START  	0X0800F000
#define USER_ID_PAGE1_ADDR_START  	0X0800F400
#define USER_ID_PAGE2_ADDR_START  	0X0800F800
#define ADMIN_ID_PAGE4_ADDR_START   0X0800FC00

#define ARRAY_SIZE  32
#define MAP_SIZE  (sizeof(uint32_t)*8)
#define USER_ID_MAX     95   //user  0-95
#define ADMIN_ID_MAX    99	 //admin 96-99
 
typedef int8_t (*Fun)(int8_t id); 


typedef enum
{
	TOUCH_PSWD = 1,
	RFID_PSWD,
	FINGER_PSWD
}pswd_type_t;

typedef struct{
	uint16_t flag;
	uint8_t id;
	pswd_type_t type;
	uint16_t len;
	uint8_t password[21];
	uint8_t reserved[5];
}id_infor_t;


typedef enum{
 
	SAVE_OK = 0,
	SAVE_FAIL
}FLASH_STATUS;

typedef enum{
	NORMAL=0,
	SECURITY
}work_mode_t;

typedef enum{
	USER_ID,
	ADMIN_ID
}ID_range_t;


typedef struct{
	uint16_t flag;
	work_mode_t work_mode;
	uint32_t  index_map[4];//每一个bit代表一个id位置，1表示此id内有数据，0 表示id内无数据
}lock_infor_t;

extern lock_infor_t lock_infor;

void Index_Init(void);
uint8_t Get_id_Number(void);
uint8_t Get_User_id_Number(void);
uint8_t Get_Admin_id_Number(void);
uint8_t Get_id_Number(void);

int8_t Find_Next_User_Null_ID_Add(int8_t id);
int8_t Find_Next_User_Null_ID_Dec(int8_t id);
int8_t Find_Next_User_ID_Add(int8_t id);
int8_t Find_Next_User_ID_Dec(int8_t id);
int8_t Find_Next_Admin_Null_ID_Add(int8_t id);
int8_t Find_Next_Admin_Null_ID_Dec(int8_t id);
int8_t Find_Next_Admin_ID_Add(int8_t id);
int8_t Find_Next_Admin_ID_Dec(int8_t id);
int8_t Find_Next_ID(int8_t id);

int8_t Delect_One_ID(uint8_t id);
void Delete_All_Admin_ID(void);
void Delete_All_User_ID(void);
void Delete_All_ID(void);

int8_t Add_One_ID(uint8_t id, id_infor_t id_infor);

/*
* range : 01比较user，02：比较admin 03：比较全部
* flag : 0 需要比较len
* flag : 1 不需要比较len
*return 0没有匹配的ID，大于0，id为库中匹配的id
*/
int8_t Compare_To_Flash_id(pswd_type_t type, uint8_t len, char *search, uint8_t flag, uint8_t rage);
int8_t CompareReverse_To_Flash_id(pswd_type_t type, uint8_t len, char *search,uint8_t flag);
//int8_t Compare_To_Flash_Admin_id(pswd_type_t type, uint8_t len, char *search, uint8_t flag);
int8_t Get_Finger_From_InterIndex(uint16_t d);
int8_t Get_Finger_User_From_InterIndex(uint16_t d);
int8_t Get_Finger_Admin_From_InterIndex(uint16_t d);
int8_t Get_Finger_Admin_Num(void);

void Set_Work_Mode(work_mode_t mode);
int Read_Select_ID(uint8_t id, id_infor_t *pID);
#endif
