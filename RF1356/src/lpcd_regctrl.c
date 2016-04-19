/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/

/*********************************************************************/
/* 	FM175XX扩展寄存器关函数											 */
/* 	主要功能:														 */
/* 		1.实现扩展寄存器的读、写、修改等配置						 */
/* 	编制:罗挺松 													 */
/* 	编制时间:2014年7月18日											 */
/* 																	 */
/*********************************************************************/

#include "lpcd_reg.h"
#include "lpcd_regctrl.h"


//***********************************************
//函数名称：GetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData)
//函数功能：读取扩展寄存器值
//入口参数：ExtRegAddr:扩展寄存器地址   ExtRegData:读取的值
//出口参数：unsigned char  TRUE：读取成功   FALSE:失败
//***********************************************

unsigned char GetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData)
{
	unsigned char res;
	unsigned char addr,regdata;

	addr = JREG_EXT_REG_ENTRANCE;
	regdata = JBIT_EXT_REG_RD_ADDR + ExtRegAddr;//JBIT_EXT_REG_RD_DATA   JBIT_EXT_REG_RD_ADDR
	RF1356_MasterWriteData(addr,regdata);
	

	addr = JREG_EXT_REG_ENTRANCE;
	res = RF1356_MasterReadData2(addr,&regdata);
	if (res == FALSE) 
	return FALSE;
	*ExtRegData = regdata;

	return TRUE;	
}

//***********************************************
//函数名称：SetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData)
//函数功能：写扩展寄存器
//入口参数：ExtRegAddr:扩展寄存器地址   ExtRegData:要写入的值
//出口参数：unsigned char  TRUE：写成功   FALSE:写失败
//***********************************************
unsigned char SetReg_Ext(unsigned char ExtRegAddr,unsigned char ExtRegData)
{
	unsigned char addr,regdata;

	addr = JREG_EXT_REG_ENTRANCE;
	regdata = JBIT_EXT_REG_WR_ADDR + ExtRegAddr;
	RF1356_MasterWriteData(addr,regdata);

	addr = JREG_EXT_REG_ENTRANCE;
	regdata = JBIT_EXT_REG_WR_DATA + ExtRegData;
	RF1356_MasterWriteData(addr,regdata);

	return TRUE;	
}

//*******************************************************
//函数名称：ModifyReg_Ext(unsigned char ExtRegAddr,unsigned char* mask,unsigned char set)
//函数功能：寄存器位操作
//入口参数：ExtRegAddr:目标寄存器地址   mask:要改变的位  
//         set:  0:标志的位清零   其它:标志的位置起
//出口参数：unsigned char  TRUE：写成功   FALSE:写失败
//********************************************************
unsigned char ModifyReg_Ext(unsigned char ExtRegAddr,unsigned char mask,unsigned char set)
{
   	unsigned char status;
	unsigned char regdata;
	
	status = GetReg_Ext(ExtRegAddr,&regdata);
	if(status == TRUE)
	{
		if(set)
		{
			regdata |= mask;
		}
		else
		{
			regdata &= ~(mask);
		}
	}
	else
		return FALSE;

	status = SetReg_Ext(ExtRegAddr,regdata);
	return status;
}
