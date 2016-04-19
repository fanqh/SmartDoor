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
/* 	FM175XX��չ�Ĵ����غ���											 */
/* 	��Ҫ����:														 */
/* 		1.ʵ����չ�Ĵ����Ķ���д���޸ĵ�����						 */
/* 	����:��ͦ�� 													 */
/* 	����ʱ��:2014��7��18��											 */
/* 																	 */
/*********************************************************************/

#include "lpcd_reg.h"
#include "lpcd_regctrl.h"


//***********************************************
//�������ƣ�GetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData)
//�������ܣ���ȡ��չ�Ĵ���ֵ
//��ڲ�����ExtRegAddr:��չ�Ĵ�����ַ   ExtRegData:��ȡ��ֵ
//���ڲ�����unsigned char  TRUE����ȡ�ɹ�   FALSE:ʧ��
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
//�������ƣ�SetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData)
//�������ܣ�д��չ�Ĵ���
//��ڲ�����ExtRegAddr:��չ�Ĵ�����ַ   ExtRegData:Ҫд���ֵ
//���ڲ�����unsigned char  TRUE��д�ɹ�   FALSE:дʧ��
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
//�������ƣ�ModifyReg_Ext(unsigned char ExtRegAddr,unsigned char* mask,unsigned char set)
//�������ܣ��Ĵ���λ����
//��ڲ�����ExtRegAddr:Ŀ��Ĵ�����ַ   mask:Ҫ�ı��λ  
//         set:  0:��־��λ����   ����:��־��λ����
//���ڲ�����unsigned char  TRUE��д�ɹ�   FALSE:дʧ��
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
