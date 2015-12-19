#include "index.h"
#include "stm32f0xx_flash.h"
#include "stm32f0xx_flash.h"
#include "string.h"

//index_mapping_t index_struct;
lock_infor_t lock_infor;

#define COLUMN 16
#define ROW    8


//static FLASH_STATUS Index_Save(void);
static int Flash_Read_Byte4(uint32_t addr, uint32_t *des, uint16_t len);
static FLASH_STATUS Flash_Write(uint32_t addr, uint32_t *src, uint16_t len);
static FLASH_STATUS id_infor_Write(uint32_t addr, id_infor_t id_data);



void Index_Init(void)
{
		uint8_t i;

		lock_infor = (*(lock_infor_t*)INDEX_ADDR_START);
		if((lock_infor.flag==0xffff)||(lock_infor.index_map[3]&0xff00))
		{

			for(i=0; i<4; i++)
				lock_infor.index_map[i] = 0x00;
			lock_infor.work_mode = NORMAL;
			Index_Save();
		}	
}

static int Flash_Read_Byte4(uint32_t addr, uint32_t *des, uint16_t len)
{
	uint16_t i;
	uint32_t *p;
	
	if(len==0)
		return -1;
	p = (uint32_t*)addr;
	for(i=0; i<(len+3)/4; i++)
	{
		*des = *p;
		des ++;
		p ++;
	}
	return len;
}

static int Read_Select_ID(uint8_t id, id_infor_t *pID)
{
		uint32_t addr;
	
		if((id==0)||(id>=100))
				return -1;
		if(id<=95)
			addr = USER_ID_PAGE0_ADDR_START + (id-1) * sizeof(id_infor_t);
		else 
			addr = ADMIN_ID_PAGE4_ADDR_START + (id-96)*sizeof(id_infor_t);
		*pID = *(id_infor_t*) addr;	
		return sizeof(id_infor_t);
}

 FLASH_STATUS Index_Save(void)
{
	lock_infor_t temp;
	
	temp = (*(lock_infor_t*)INDEX_ADDR_START);
	if(temp.flag!=0xffff)
	{
			FLASH_Unlock();
			FLASH_ErasePage(INDEX_ADDR_START);
			FLASH_Lock();
	}
		
	lock_infor.flag =0x01;
	return Flash_Write(INDEX_ADDR_START, (uint32_t*)&lock_infor, sizeof(lock_infor_t));
}

static void Index_Read(void)
{
			uint8_t i;
	
			lock_infor = (*(lock_infor_t*)INDEX_ADDR_START);
			if(lock_infor.flag==0xffff)
			{

				for(i=0; i<4; i++)
					lock_infor.index_map[i] = 0x00;
				lock_infor.work_mode = NORMAL;
				Index_Save();
			}
}

/*

typedef struct{
	uint16_t flag;
	work_mode_t work_mode;
	index_mapping_t index_map;
}lock_infor_t;

*/

int8_t Add_Index(uint8_t id)
{
	uint8_t m,n;
	
	if(id>99)
		return -1;
	if((id>=96)&&(id<=99))
		lock_infor.work_mode = SECURITY;
	
	m = (id-1) / MAP_SIZE;
	n = (id-1) % MAP_SIZE;
	
	lock_infor.index_map[m] |= 1<<n;
	Index_Save();
	return 1;
		
}

int8_t Delect_Index(uint8_t id)
{
	uint8_t m,n;
	
	if(id>99)
		return -1;
	if((id>=96)&&(id<=99))
		lock_infor.work_mode = SECURITY;
	
	m = (id-1) / MAP_SIZE;
	n = (id-1) % MAP_SIZE;
	
	lock_infor.index_map[m] &= (~(1<<n));
	Index_Save();
	return 1;
		
}

/*
int8_t Find_Next_ID(int8_t id)
{
	uint8_t i,j;
	uint8_t m,n;

	
	if((id>=99)||(id<1))
	{
		m = 0;
		n = 0;
	}
	else
	{
		m = (id-1) / MAP_SIZE;
		n = (id-1) % MAP_SIZE;
	}
	
	for(j=m; j<=(ADMIN_ID_MAX)/ARRAY_SIZE; j++)
	{	
		if(n==MAP_SIZE-1)
		{
			n = 1;
			++j;
		}
		else
			n++;
		for(i=n; i<ARRAY_SIZE; i++)
		{
			if(lock_infor.index_map[j]&(1<<(i-1)))
				return j*MAP_SIZE + i ;
		}
	}
	return -1;	//无空的id
}

*/

int8_t Find_Next_ID(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>=100)||(id<1))
		t =1;
	else
		t = id+1;
	while(t<100)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n)))
			return  m*MAP_SIZE + n +1;
		t++;
	}
	return -1;
}

/*id 0 -- 96 */
int8_t Find_Next_User_Null_ID_Add(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>95)||(id<1))
		t =1;
	else
		t = id+1;
	while(t<96)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n))==0)
			return  m*MAP_SIZE + n +1;
		t++;
	}
	return -2;
				
}

/* 0--96 */
int8_t Find_Next_User_Null_ID_Dec(int8_t id)
{
	
	uint8_t t,m,n;
	
	if((id>=96)||(id<1))
		t =95;
	else
		t = id-1;
	while(t)	
	{		
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n))==0)
			return  m*MAP_SIZE + n +1;
		t--;
	}
	return -2;	
}

int8_t Find_Next_User_ID_Add(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>95)||(id<1))
		t =1;
	else
		t = id+1;
	while(t<96)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n)))
			return  m*MAP_SIZE + n +1;
		t++;
	}
	return -1;
		
}

int8_t Find_Next_User_ID_Dec(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>=96)||(id<1))
		t =95;
	else
		t = id-1;
	while(t)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n)))
			return  m*MAP_SIZE + n +1;
		t--;
	}	
	return -1;	//无数据	
}


#if 1

/*id 96 -- 99 */
int8_t Find_Next_Admin_Null_ID_Add(int8_t id)
{
	
	uint8_t t,m,n;
	
	if((id<96)||(id>99))
		t =96;
	else
		t = id+1;
	while(t<100)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n))==0)
			return  m*MAP_SIZE + n +1;
		t++;
	}
	return -2;	//数据已满		
}



/*  */
int8_t Find_Next_Admin_Null_ID_Dec(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>99)||(id<96))
		t =99;
	else
		t = id-1;
	while(t>=96)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n))==0)
			return  m*MAP_SIZE + n +1;
		t--;
	}
	return -2;				
}

int8_t Find_Next_Admin_ID_Add(int8_t id)
{
	
	uint8_t t,m,n;
	
	if((id<96)||(id>99))
		t =96;
	else
		t = id+1;
	while(t<100)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n)))
			return  m*MAP_SIZE + n +1;
		t++;
	}
	return -1;	//数据已满		
}

int8_t Find_Next_Admin_ID_Dec(int8_t id)
{
	uint8_t t,m,n;
	
	if((id>99)||(id<96))
		t =99;
	else
		t = id-1;
	while(t>=96)	
	{
		m = (t-1) / MAP_SIZE;
		n = (t-1) % MAP_SIZE;
		if((lock_infor.index_map[m]&(1<<n)))
			return  m*MAP_SIZE + n +1;
		t--;
	}
	return -1;		
}

#endif

//FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);

static FLASH_STATUS Flash_Write(uint32_t addr, uint32_t *src, uint16_t len)
{
	uint16_t i,count=0;
	
	if(len==0)
		return SAVE_FAIL;
	FLASH_Unlock();
	for(i=0; i<(len+3)/4; i++)
	{
		while((FLASH_ProgramWord((uint32_t) addr, *src)!=FLASH_COMPLETE)&&(count<10))
		{
			count++;
		//	FLASH_Lock();
			//return SAVE_FAIL;
		}
		if(count>=10)
		{
			FLASH_Lock();
			return SAVE_FAIL;
		}
		addr += 4;
		src++;
	}
	FLASH_Lock();
	return SAVE_OK;
}

static FLASH_STATUS id_infor_Write(uint32_t addr, id_infor_t id_data)
{
	return Flash_Write( addr, (uint32_t*)&id_data, sizeof(id_infor_t));
}


id_infor_t Sector_data[32];

FLASH_STATUS id_infor_Save(uint8_t id, id_infor_t id_struct)
{
	id_infor_t  id_data, *p;

	uint32_t sector_addr,addr;
	uint16_t i;
	
	if((id>=1)&&(id<=32))
	{
		sector_addr = USER_ID_PAGE0_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + (id-1) * sizeof(id_infor_t);
	}
	else if((id>=33)&&(id<=64))
	{
		sector_addr = USER_ID_PAGE1_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + (id-1) * sizeof(id_infor_t);
	}
	else if((id>=65)&&(id<=95))
	{
		sector_addr = USER_ID_PAGE2_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + (id-1) * sizeof(id_infor_t);
	}
	else if((id>=96)&&(id<=99))
	{
		sector_addr = ADMIN_ID_PAGE4_ADDR_START;
		addr = ADMIN_ID_PAGE4_ADDR_START + (id-96)* sizeof(id_infor_t);
	}
	else
		return SAVE_FAIL;
	
	id_data = *(id_infor_t*)addr;
	if(id_data.flag==0xffff)
	{
		id_struct.flag = 0x01;
//		FLASH_ProgramWord(addr, addr);
		id_infor_Write(addr, id_struct);
		return SAVE_OK;
	}
	else
	{
		p = (id_infor_t*)sector_addr;
		if((id>=1)&&(id<=95))
		{
			for(i=0; i<32; i++)
			{
				Sector_data[i] = *p;
				p++;
			}
			FLASH_Unlock();
			FLASH_ErasePage(sector_addr);
			FLASH_Lock();
			Sector_data[id-1] = id_struct;
			Sector_data[id-1].flag = 0x01;
			return Flash_Write(sector_addr, (uint32_t*)Sector_data ,sizeof(id_infor_t)*32);
		}
		else
		{
			for(i=0; i<4; i++)
			{
				Sector_data[i] = *p;
				p++;
			}
			FLASH_Unlock();
			FLASH_ErasePage(sector_addr);
			FLASH_Lock();
			Sector_data[id-96] = id_struct;
			Sector_data[id-96].flag = 0x01;
			return Flash_Write(sector_addr, (uint32_t*)Sector_data ,sizeof(id_infor_t)*4);
		}
//		return SAVE_OK;	
	}
}



/*
typedef struct{
	uint16_t flag;
	work_mode_t work_mode;
	index_mapping_t index_map;
}lock_infor_t;
*/

void Erase_All_User_id(void)
{
	uint8_t i;
	FLASH_Unlock();
	FLASH_ErasePage(USER_ID_PAGE0_ADDR_START);
	FLASH_ErasePage(USER_ID_PAGE1_ADDR_START);
	FLASH_ErasePage(USER_ID_PAGE2_ADDR_START);
	FLASH_Lock();
	for(i=1; i<96; i++)
		lock_infor.index_map[(i-1)/MAP_SIZE] &= (~(1<<((i-1)%MAP_SIZE)));
	Index_Save();
}
void Erase_All_Admin_id(void)
{
	uint8_t id;
	
	lock_infor.work_mode = NORMAL;
	FLASH_Unlock();
	FLASH_ErasePage(ADMIN_ID_PAGE4_ADDR_START);
	FLASH_Lock();
	for(id=96; id<100; id++)
	{
		lock_infor.index_map[(id-1)/MAP_SIZE] &= (~(1<<((id-1)%MAP_SIZE)));
	}
	Index_Save();
}
void Erase_All_id(void)
{
		uint8_t i;
	
	FLASH_Unlock();
	FLASH_ErasePage(USER_ID_PAGE0_ADDR_START);
	FLASH_ErasePage(USER_ID_PAGE1_ADDR_START);
	FLASH_ErasePage(USER_ID_PAGE2_ADDR_START);
	FLASH_ErasePage(ADMIN_ID_PAGE4_ADDR_START);
	FLASH_Lock();
	
	lock_infor.work_mode = NORMAL;
	for(i=0; i<4; i++)
		lock_infor.index_map[i]=0;
	Index_Save();
}

uint8_t Get_id_Number(void)
{
	uint8_t i;
	uint8_t num = 0;
	for(i=1; i<100; i++)
	{
		if(lock_infor.index_map[(i-1)/MAP_SIZE] & ((1<<((i-1)%MAP_SIZE))))
			num++;
	}
	return num;
}

uint8_t Get_User_id_Number(void)
{
	uint8_t i;
	uint8_t num = 0;
	for(i=1; i<96; i++)
	{
		if(lock_infor.index_map[(i-1)/MAP_SIZE] & ((1<<((i-1)%MAP_SIZE))))
			num++;
	}
	return num;
}
uint8_t Get_Admin_id_Number(void)
{
	uint8_t i;
	uint8_t num = 0;
	for(i=96; i<100; i++)
	{
		if(lock_infor.index_map[(i-1)/MAP_SIZE] & ((1<<((i-1)%MAP_SIZE))))
			num++;
	}
	return num;
}


/*
* flag : 0 需要比较len
* flag : 1 不需要比较len
*/
int8_t Compare_To_Flash_id(pswd_type_t type, uint8_t len, char *search,uint8_t flag)
{		
		uint8_t i, num,j,*p;
		int8_t id=0;
		id_infor_t  id_infor;
		
		num = Get_id_Number();
		if(num==0)
			return 0;
		for(i=0; i<num; i++)
		{
			id = Find_Next_ID(id);
		  if(id==-1)
				return 0;
//			printf("%d\r\n",id);
			Read_Select_ID(id, &id_infor);
//			p = id_infor.password;
//			for(j=0;j<id_infor.len;j++)
//			{
//				printf("%c",*p++);
//			}
//			printf("\r\n");
			if((type==id_infor.type)&&((flag==1)||(len==id_infor.len))&&(NULL!=strstr(search, id_infor.password)))
				return id;
		}
		return 0;
}

int8_t CompareReverse_To_Flash_id(pswd_type_t type, uint8_t len, char *search,uint8_t flag)
{		
		uint8_t i, num,j,*p;
		int8_t id=0;
		id_infor_t  id_infor;
		
		num = Get_id_Number();
		if(num==0)
			return 0;
		for(i=0; i<num; i++)
		{
			id = Find_Next_ID(id);
		  if(id==-1)
				return 0;
//			printf("%d\r\n",id);
			Read_Select_ID(id, &id_infor);
			p = id_infor.password;
//			for(j=0;j<id_infor.len;j++)
//			{
//				printf("%c",*p++);
//			}
//			printf("\r\n");
			if((type==id_infor.type)&&((flag==1)||(len==id_infor.len))&&(NULL!=strstr(id_infor.password, search)))
				return id;
		}
		return 0;
}

int8_t Compare_To_Flash_User_id(pswd_type_t type, char *search)
{		
		uint8_t i, num;
		int8_t id=0;
		id_infor_t  id_infor;
		
		num = Get_User_id_Number();
		if(num==0)
			return 0;
		for(i=0; i<num; i++)
		{
			id = Find_Next_User_ID_Add(id);
		  if(id==-1)
				return 0;
			Read_Select_ID(id, &id_infor);
			if((type==id_infor.type)&&(NULL!=strstr(search, id_infor.password)))
				return id;
		}
		return 0;
}


/*
* flag : 0 需要比较len
* flag : 1 不需要比较len
*/
int8_t Compare_To_Flash_Admin_id(pswd_type_t type, uint8_t len, char *search, uint8_t flag)
{		
		uint8_t i, num;
		int8_t id=95;
		id_infor_t  id_infor;
		
		num = Get_Admin_id_Number();
		if(num==0)
			return 0;
		for(i=0; i<num; i++)
		{
			id = Find_Next_Admin_ID_Add(id);
		  if(id==-1)
				return 0;
			Read_Select_ID(id, &id_infor);
			if((type==id_infor.type)&&((flag==1)||(len==id_infor.len))&&(NULL!=strstr(search, id_infor.password)))
				return id;
		}
		return 0;
}


