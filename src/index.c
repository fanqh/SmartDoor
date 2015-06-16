#include "index.h"
#include "stm32f0xx_flash.h"
#include "stm32f0xx_flash.h"
#include "string.h"

//index_mapping_t index_struct;
lock_infor_t lock_infor;

#define COLUMN 16
#define ROW    8



void Index_Init(void)
{
			Index_Read();
}

int Flash_Read_Byte4(uint32_t addr, uint32_t *des, uint16_t len)
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

int Read_Select_ID(uint8_t id, id_infor_t *pID)
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
	uint32_t temp;
	
	temp = (*(uint16_t*)INDEX_ADDR_START);
	if(temp!=0xffff)
			FLASH_ErasePage(INDEX_ADDR_START);
		
	lock_infor.flag =0x01;
	return Flash_Write(INDEX_ADDR_START, (uint32_t*)&lock_infor, sizeof(lock_infor_t));
}

void Index_Read(void)
{
			lock_infor = (*(lock_infor_t*)INDEX_ADDR_START);
			if((lock_infor.index_map.x==0xffff) || (lock_infor.index_map.y ==0xffff))
			{
				lock_infor.index_map.x = 0;
				lock_infor.index_map.y = 0;
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
	m = (id-1)/COLUMN ;	//raw
	n = (id-1)%COLUMN ;	//column
	
	lock_infor.index_map.x |= 1<<n;
	lock_infor.index_map.y |= 1<<m;
	Index_Save();
		
}


int8_t Find_Next_ID(int8_t id)
{
	uint8_t i,j;
	uint8_t m,n,temp;
	
	if((id>=99)||(id<1))
	{
		m = 0;
		n = 0;
	}
	else
	{
		m = (id-1)/COLUMN ;	//raw
		n = (id-1)%COLUMN ;	//column
	}
	
	for(j=m; j<ROW; j++)
	{
		if(n==COLUMN-1)
			++j;
		if((j!=m)||(n==(COLUMN-1))||(id>=99)||(id==0))
			temp= 0;
		else 
			temp = n+1;
		
		if((lock_infor.index_map.y & (1<<j)))
		{
			for(i= temp; i<COLUMN; i++)
			{
				if((lock_infor.index_map.x & (1<<i)))
					return (j)*COLUMN + i + 1;
			}
		}

	}
	return -1;	//无数据	
}

/*id 0 -- 96 */
int8_t Find_Next_User_Null_ID(int8_t id)
{
	uint8_t i,j;
	uint8_t m,n;
	uint8_t temp;
	

	if((id>=96)||(id<1))
	{
		m = 0;
		n = 0;
	}
	else
	{
		m = (id-1)/COLUMN ;	//raw
		n = (id-1)%COLUMN ;	//column
	}
	
	for(j=m; j<ROW; j++)
	{
		if(n==COLUMN-1)
			++j;
		if((j!=m)||(n==(COLUMN-1))||(id>=96)||(id==0))
			temp= 0;
		else 
			temp = n+1;
		for(i= temp; i<COLUMN; i++)
		{
			if(((lock_infor.index_map.y & (1<<j))==0) && ((lock_infor.index_map.x & (1<<i))==0))
				return j*COLUMN + i + 1;
		}
	}
	return -2;	//数据已满		
}

/* 0--96 */
int8_t Find_Next_User_Null_ID_Dec(int8_t id)
{
	int8_t i,j;
	uint8_t m,n,temp;
	
	if((id<=1)||(id>=96)) /* 后面没有数据了 */
	{
		m = 5;
		n = 15;
	}
	else
	{
		m = (id-1)/COLUMN ;			//raw
		n = (id-1)%COLUMN ;			//column
	}
	
	for(j=m; j>=0; j--)
	{
		if(n==0)
			--j;
		if((j!=m)||(n==0))
			temp= COLUMN-1;
		else
			temp = n-1;
		for(i= temp; i>=0; i--)
		{
			if(((lock_infor.index_map.y & (1<<j))==0) && ((lock_infor.index_map.x & (1<<i))==0))
				return j*COLUMN + i + 1;
		}
	}
	return -2;	//数据已满		
}

int8_t Find_Next_User_ID(int8_t id)
{
	uint8_t i,j;
	uint8_t m,n,temp;
	
	if((id>=96)||(id<1))
	{
		m = 0;
		n = 0;
	}
	else
	{
		m = (id-1)/COLUMN ;	//raw
		n = (id-1)%COLUMN ;	//column
	}
	
	for(j=m; j<ROW; j++)
	{
		if(n==COLUMN-1)
			++j;
		if((j!=m)||(n==(COLUMN-1))||(id>=96)||(id==0))
			temp= 0;
		else 
			temp = n+1;
		
		if((lock_infor.index_map.y & (1<<j)))
		{
			for(i= temp; i<COLUMN; i++)
			{
				if((lock_infor.index_map.x & (1<<i)))
					return (j)*COLUMN + i + 1;
			}
		}

	}
	return -1;	//无数据	
}

int8_t Find_Next_User_ID_Dec(int8_t id)
{
	int8_t i,j;
	uint8_t m,n,temp;
	
	if((id<=1)||(id>=96)) /* 后面没有数据了 */
	{
		m = 5;
		n = 15;
	}
	else
	{
		m = (id-1)/COLUMN ;			//raw
		n = (id-1)%COLUMN ;			//column
	}
	
	for(j=m; j>=0; j--)
	{
		if(n==0)
			--j;
		if((j!=m)||(n==0))
			temp= COLUMN-1;
		else
			temp = n-1;
		
		if((lock_infor.index_map.y & (1<<j)))
		{
			for(i= temp; i>=0; i--)
			{
				if((lock_infor.index_map.x & (1<<i)))
					return (j)*COLUMN + i + 1;
			}
		}
	}
	return -1;	//无数据	
}


#if 1

/*id 96 -- 99 */
int8_t Find_Next_Admin_Null_ID(int8_t id)
{
	uint8_t i,j;
	uint8_t m,n;
	uint8_t temp;
	
	if((id<96)||(id>99))
		id = 95;
	
	m = (id-1)/COLUMN ;	//raw
	n = (id-1)%COLUMN ;	//column
	
	for(j=m; j<ROW; j++)
	{
		if(n==COLUMN-1)
			++j;
		if((j!=m)||(n==(COLUMN-1)))
			temp= 0;
		else 
			temp = n+1;
		for(i= temp; i<COLUMN; i++)
		{
			if(((lock_infor.index_map.y & (1<<j))==0) && ((lock_infor.index_map.x & (1<<i))==0))
				return j*COLUMN + i + 1;
		}
	}
	return -2;	//数据已满		
}



/*  */
int8_t Find_Next_Admin_Null_ID_Dec(int8_t id)
{
	int8_t i,j;
	uint8_t m,n,temp;
	
	if((id<=96)||(id>100)) /* 后面没有数据了 */
		id = 100;

	m = (id-1)/COLUMN ;			//raw
	n = (id-1)%COLUMN ;			//column
	
	for(j=m; j>=0; j--)
	{
		if(n==0)
			--j;
		if((j!=m)||(n==0))
			temp= COLUMN-1;
		else
			temp = n-1;
		for(i= temp; i>=0; i--)
		{
			if(((lock_infor.index_map.y & (1<<j))==0) && ((lock_infor.index_map.x & (1<<i))==0))
				return j*COLUMN + i + 1;
		}
	}
	return -2;	//数据已满		
}

int8_t Find_Next_Admin_ID(int8_t id)
{
	int8_t i,j;
	uint8_t m,n,temp;
	
	if((id<96)||(id>99))
		id = 95;
	
	m = (id-1)/COLUMN ;	//raw
	n = (id-1)%COLUMN ;	//column
	
	for(j=m; j<ROW; j++)
	{
		if((j!=m)||(n==COLUMN-1))
			temp= 0;
		else
			temp = n+1;
		if((lock_infor.index_map.y & (1<<j)))
		{
			for(i= temp; i<COLUMN; i++)
			{
				if((lock_infor.index_map.x & (1<<i)))
					return (j)*COLUMN + i + 1;
			}
		}

	}
	return -1;	//无数据	
}

int8_t Find_Next_Admin_ID_Dec(int8_t id)
{
	int8_t i,j;
	uint8_t m,n,temp;
	
	if((id<96)||(id>99))
		id = 95;
	
	m = (id-1)/COLUMN ;	//raw
	n = (id-1)%COLUMN ;	//column
	
	for(j=m; j>=0; j--)
	{
		if((j!=m)||(n==0))
			temp= COLUMN-1;
		else
			temp = n-1;
		if((lock_infor.index_map.y & (1<<j)))
		{
			for(i= temp; i>=0; i--)
			{
				if((lock_infor.index_map.x & (1<<i)))
					return (j)*COLUMN + i + 1;
			}
		}
	}
	return -1;	//无数据	
}

#endif

//FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);

FLASH_STATUS Flash_Write(uint32_t addr, uint32_t *src, uint16_t len)
{
	uint16_t i;
	
	if(len==0)
		return SAVE_FAIL;
	for(i=0; i<(len+3)/4; i++)
	{
		if(FLASH_ProgramWord((uint32_t) addr, *src)!=FLASH_COMPLETE)
			return SAVE_FAIL;
		addr += 4;
		src++;
	}
	return SAVE_OK;
}

FLASH_STATUS id_infor_Write(uint32_t addr, id_infor_t id_data)
{
	return Flash_Write( addr, (uint32_t*)&id_data, sizeof(id_infor_t));
}



FLASH_STATUS id_infor_Save(uint8_t id, id_infor_t id_struct)
{
	id_infor_t id_data, Sector_data[32],*p;
	uint32_t sector_addr,addr;
	uint16_t i;
	
	if((id>=1)&&(id<=32))
	{
		sector_addr = USER_ID_PAGE0_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + id * sizeof(id_infor_t);
	}
	else if((id>=33)&&(id<=64))
	{
		sector_addr = USER_ID_PAGE1_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + id * sizeof(id_infor_t);
	}
	else if((id>=65)&&(id<=95))
	{
		sector_addr = USER_ID_PAGE2_ADDR_START;
		addr = USER_ID_PAGE0_ADDR_START + id * sizeof(id_infor_t);
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
		id_data.flag = 0x01;
		FLASH_ProgramWord(addr, addr);
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
			FLASH_ErasePage(sector_addr);
			Sector_data[id-1] = id_data;
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
			FLASH_ErasePage(sector_addr);
			Sector_data[id-96] = id_data;
			Sector_data[id-96].flag = 0x01;
			return Flash_Write(sector_addr, (uint32_t*)Sector_data ,sizeof(id_infor_t)*4);
		}
		//return SAVE_OK;	
	}
}

uint8_t Get_id_Number(void)
{
	uint8_t i,j;
	uint8_t num = 0;
	for(i=0; i<COLUMN; i++)
	{
		if(lock_infor.index_map.x && (1<<i))
		{
			for(j=0; j<ROW; j++)
			{
				if(lock_infor.index_map.y && (1<<j))
					num++;
			}
		}
	}
	return num;
}

uint8_t Get_User_id_Number(void)
{
		return (Get_id_Number() - Get_Admin_id_Number());
}
uint8_t Get_Admin_id_Number(void)
{
		uint8_t i;
		uint8_t num = 0;
	
		if(lock_infor.index_map.y & (1<<7))  //96 为j = 7; i= 0;
		{
			for(i=0; i<4; i++)
			{
				if(lock_infor.index_map.x & (1<<i))
					num ++;
			}
		}
		if(num>4)
			num = 0;
		return num;
}

int8_t Compare_To_Flash_id(pswd_type_t type, char *search)
{		
		uint8_t i, num;
		int8_t id;
		id_infor_t  id_infor;
		
		num = Get_id_Number();
		if(num==0)
			return 0;
		for(i=0; i<num; i++)
		{
			id = Find_Next_ID(id);
		  if(id==-1)
				return 0;
			Read_Select_ID(id, &id_infor);
			if((type==id_infor.type)&&(NULL!=strstr(search, id_infor.password)))
				return id;
		}
		return 0;
}



