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
/* 	FM175XX LPCD API������											 */
/* 	��Ҫ����:														 */
/* 		1.ʵ��LPCD���API����										 */
/* 	����:��ͦ�� 													 */
/* 	����ʱ��:2014��7��18��											 */
/* 																	 */
/*********************************************************************/

#include "lpcd_api.h"
#include "delay.h" 

#define IF_ERR_THEN_RETURN if (ret == FALSE) return FALSE
#define ADC_REFERNCE_MIN 0x0	//ʵ�ʲ�������ֵ��С
#define ADC_REFERNCE_MAX 0x7F	//ʵ�ʵ���ֵ���

typedef  unsigned char uchar;  
//********************************************************************
//�ڲ������б�
//********************************************************************
static unsigned char WaitForLpcdIrq(unsigned char IrqType);
static unsigned char ReadLpcdADCResult(unsigned char *ADCResult);
static unsigned char CalibraReadADCResult(unsigned char *ADCResult);
// static unsigned char LpcdSetPGA_GainReduce(unsigned char *ADCResult);
// static unsigned char LpcdSetPGA_GainAmplify(unsigned char *ADCResult);
static unsigned char LpcdSet_PGA(unsigned char *GainCalibraFlag, unsigned char *ADCResult);
static unsigned char LpcdSet_ADCRefvoltage(unsigned char *CalibraFlag, unsigned char *ADCResult);
unsigned char  LpcdSet_DetectSensitive(float Sensitive);

//********************************************************************
//���̱����б�
//********************************************************************
unsigned char T3ClkDivK ;
unsigned char LpcdBiasCurrent ;//3bit  //����Ƭ����ָ���������config�ļ����趨
unsigned char LpcdGainReduce;//2bit
unsigned char LpcdGainAmplify;//3bit
unsigned char LpcdADCRefernce;//7bit 

unsigned char Timer1Cfg;//4bit
unsigned char Timer2Cfg;//5bit
unsigned char Timer3Cfg;//5bit

unsigned char ADCResultFullScale;	//T3������ADCResult��Ϣ
unsigned char ADCResultThreshold;	//�����ȣ����ó����ֵ
unsigned char LpcdThreshold_L;		//LPCD���ȵ���ֵ
unsigned char LpcdThreshold_H;		//LPCD���ȸ���ֵ
unsigned char ADCResultCenter;		//LPCD�������ĵ�
unsigned char LpcdADCResult[10];	//Lpcd������Ϣ�������󴥷��ж�

//***********************************************
//�������ƣ�LpcdParamInit()
//�������ܣ�LPCD������ʼ��
//��ڲ�����
//���ڲ�����uchar  TRUE����ȡ�ɹ�   FALSE:ʧ��
//***********************************************
unsigned char LpcdParamInit()
{
	LpcdBiasCurrent = LPCD_BIAS_CURRENT ; //����Ƭ����ָ���������config�ļ����趨
 	LpcdGainReduce = 0x3;			//1x
 	LpcdGainAmplify = 0x0;			//1x
	LpcdADCRefernce = 0;
	Timer1Cfg = TIMER1_CFG;
	Timer2Cfg = TIMER2_CFG;
  Timer3Cfg = TIMER3_CFG;
	
	if (Timer3Cfg > 0xF) //Timer3Cfg�õ�5bit������ֻ��ѡ��16��Ƶ
	{
		T3ClkDivK = 2;			//16��Ƶ
		ADCResultFullScale =  ((Timer3Cfg - 1)<<3);
		ADCResultCenter = (ADCResultFullScale >>1);
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);
	}
	else if(Timer3Cfg > 0x7) //Timer3Cfg�õ�4bit������ֻ��ѡ��8��Ƶ
	{
		T3ClkDivK = 1;			//8��Ƶ
		ADCResultFullScale =  ((Timer3Cfg - 1)<<4);          //160
		ADCResultCenter = (ADCResultFullScale >>1);          //80
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);     //10
	}
	else 
	{
		T3ClkDivK = 0;			//4��Ƶ
		ADCResultFullScale =  ((Timer3Cfg - 1)<<5);
		ADCResultCenter = (ADCResultFullScale >>1);
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);
	}

	LpcdThreshold_H = ADCResultCenter + ADCResultThreshold;    //80+10=90
	LpcdThreshold_L= ADCResultCenter - ADCResultThreshold;     //80-10=70

	return TRUE;
}
//***********************************************
//�������ƣ�LpcdRegisterInit()
//�������ܣ�LPCD�Ĵ�����ʼ��
//��ڲ�����
//���ڲ�����uchar  TRUE����ʼ���ɹ�   FALSE:��ʼ��ʧ��
//***********************************************
#define  COMMIEN_DEF          0x80
#define  DIVIEN_DEF           0x80
#define ComIEnReg	0x02
#define DivIEnReg	0x03
unsigned char LpcdRegisterInit(void)
{
	unsigned char ret;
	unsigned char regdata;
	
	//�ж�����
	regdata = COMMIEN_DEF;			 //�ж�����

	RF1356_MasterWriteData(ComIEnReg,regdata);

	regdata = DIVIEN_DEF;			 
	RF1356_MasterWriteData(DivIEnReg,regdata);//����IRQ������Ϊ��׼CMOS������STatus1Reg��IRQλ�෴

	//-----------------------------------------------------------------
	//LpcdCtr1�Ĵ���
	//-----------------------------------------------------------------
	//��λLPCD�Ĵ���
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	//��λ�ſ�LPCD�Ĵ���
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	
	//ʹ��LPCD����
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_EN);
	IF_ERR_THEN_RETURN;
	
	//����LPCD�ж�
	ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_IE<<5)+JBIT_LPCD_IE);      //����LPCD�жϼĴ���״̬��ӳ��IRQ����
	IF_ERR_THEN_RETURN;	
	
	//���ý���������
	ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_IE<<5)+JBIT_LPCD_CMP_1);       //һ�μ�⵽����Ч
	//ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_DS<<5)+JBIT_LPCD_CMP_3);       //3�μ�⵽����Ч
	IF_ERR_THEN_RETURN;

	
	//-----------------------------------------------------------------
	//LpcdCtrl2�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_CTRL2,((LPCD_TX2RFEN<<4)+(LPCD_CWN<<3)+LPCD_CWP));//P������������0��7һ����������ѡ��3
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdCtrl3�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_CTRL3,LPCD_MODE<<3);        //û����ʵ������
	IF_ERR_THEN_RETURN;

	//-----------------------------------------------------------------
	//Timer1Cfg�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T1CFG,(T3ClkDivK<<4)+Timer1Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//Timer2Cfg�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T2CFG,Timer2Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//Timer3Cfg�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T3CFG,Timer3Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//VmidBdCfg�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_VMIDBD_CFG,VMID_BG_CFG);    //�������û��޸�
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//AutoWupCfg�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(AUTO_WUP_EN<<3)+AUTO_WUP_CFG);         //�����Զ�����ʱ��
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_L1�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(LpcdThreshold_L & 0x3F));
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_L2�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(LpcdThreshold_L>>6));         //���ÿ�����·�ֵ
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_H1�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(LpcdThreshold_H& 0x3F));
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_H2�Ĵ���
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_H,(LpcdThreshold_H>>6));        //���ÿ�����Ϸ�ֵ
	IF_ERR_THEN_RETURN;
	

	//-----------------------------------------------------------------
	//Auto_Wup_Cfg�Ĵ���
	//-----------------------------------------------------------------
	ret=SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(AUTO_WUP_EN<<3) + AUTO_WUP_CFG ); //�ٴ������Զ�����ʱ��
	IF_ERR_THEN_RETURN;
		
	return TRUE;
}

//***********************************************
//�������ƣ�LpcdSet_DetectSensitive()
//�������ܣ����ü��������
//��ڲ����float Sensitive��������
//���ڲ�������
//***********************************************
unsigned char  LpcdSet_DetectSensitive(float Sensitive)
{
	  unsigned char ret,Threshold;
	   Threshold =ADCResultCenter*Sensitive; 
	//΢����ֵ
			/****LpcdThreshold_H = ADCResultCenter + ADCResultThreshold;*/
			/****LpcdThreshold_L= ADCResultCenter - ADCResultThreshold;*/
			LpcdThreshold_H = ADCResultCenter + Threshold;
			LpcdThreshold_L = ADCResultCenter - Threshold;
			//-----------------------------------------------------------------
			//LpcdThreshold_L1�Ĵ���
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(LpcdThreshold_L& 0x3F));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_L2�Ĵ���
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(LpcdThreshold_L>>6));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_H1�Ĵ���
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(LpcdThreshold_H& 0x3F));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_H2�Ĵ���
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_H,(LpcdThreshold_H>>6));
			IF_ERR_THEN_RETURN;
			
			return TRUE;	
}

void close_lpcd(void)
{
    SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
}

//***********************************************
//�������ƣ�LpcdSet_ADCRefvoltage()
//�������ܣ����ú��ʵ�ADC�ο���ѹ
//��ڲ����unsigned char *CalibraFlag, unsigned char *ADCResult   ���ȼ���������ַ
//���ڲ�����uchar  TRUE�����óɹ�  FALSE:����ʧ��
//***********************************************
unsigned char  LpcdSet_ADCRefvoltage(unsigned char *CalibraFlag, unsigned char *ADCResult)
{
	  unsigned char ret;
		unsigned char ADCResult_Pre;			//������Ϣ��ǰһ��ֵ
	
	//ɨ�������
	for(LpcdADCRefernce = ADC_REFERNCE_MIN;LpcdADCRefernce < ADC_REFERNCE_MAX;LpcdADCRefernce++)
	//ͨ����ѭ������ȷ�����յĲο���ѹ��ADC���ıȽ�ֵ
	{
		//���òο���ѹֵ
		ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)<<5)+LpcdBiasCurrent&0x7);
		IF_ERR_THEN_RETURN;
		ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
		IF_ERR_THEN_RETURN;

		//���ݷ�����Ϣ
		ADCResult_Pre = *ADCResult;
		//���̶�ȡ��ǰ������Ϣ
		ret = CalibraReadADCResult(ADCResult);
		IF_ERR_THEN_RETURN;
		//==============================================
		//�㷨һ
		//==============================================
		//������ȿ�ʼ������ֵС����Ϊ��ʼ������ֵ��
		if (*ADCResult < ADCResultCenter)
		{
			//���̳ɹ�
			(*CalibraFlag) = TRUE;
			//��ǰһ�����̲ο���ѹ���ж��ĸ����ȸ��ӽ����ĵ�
			if((ADCResultCenter - *ADCResult) < (ADCResult_Pre-ADCResultCenter))
			{
				//ֱ���õ�ǰֵ��Ϊ���ĵ�
				ADCResultCenter = *ADCResult;
			}
			else
			{
				//ֱ���õ�ǰֵ��Ϊ���ĵ�
				ADCResultCenter = ADCResult_Pre;
				//�ο���ѹ����֮ǰ�Ĳο���ѹ
				LpcdADCRefernce--;
				//�������òο���ѹֵ
				ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)<<5)+LpcdBiasCurrent&0x7);
				IF_ERR_THEN_RETURN;
				ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
				IF_ERR_THEN_RETURN;

			}
			break;
		}
	}
	if(LpcdADCRefernce < ADC_REFERNCE_MAX)
		return TRUE;
	else
		return FALSE;
}

//***********************************************
//�������ƣ�LpcdSet_PGA()
//�������ܣ����ú��ʵ�PGA����
//��ڲ����unsigned char *CalibraFlag, unsigned char *ADCResult   ���ȼ���������ַ
//���ڲ�����uchar  TRUE�����óɹ�  FALSE:����ʧ��
//***********************************************
unsigned char  LpcdSet_PGA(unsigned char *GainCalibraFlag, unsigned char *ADCResult)
{
	unsigned char ret;
	//�ο���ѹֵ������С
	LpcdADCRefernce = ADC_REFERNCE_MIN;
	ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)>>1)+LpcdBiasCurrent&0x7);
	IF_ERR_THEN_RETURN;
	ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
	IF_ERR_THEN_RETURN;

	//���̶�ȡ��ǰ������Ϣ
	ret = CalibraReadADCResult(ADCResult);
	IF_ERR_THEN_RETURN;

	//ȱʡ���治��Ҫ����
	*GainCalibraFlag = TRUE;

	//�ж��Ƿ����̫խ�����̫խlpcd_gain˥��
	if  (*ADCResult < ADCResultCenter)
	{
		//������Ҫ����
		*GainCalibraFlag = FALSE;
		//*GainCalibraFlag = LpcdSetPGA_GainReduce(ADCResult);   //����PGA����˥��
		while(1)
		{
			//�����ǰ�Ѿ�����С���棬����ʧ��
			if (LpcdGainReduce == 0)
			{
				*GainCalibraFlag = FALSE;
				break;
			}
			//����˥��
			LpcdGainReduce --; 
			 
			//��������
			ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
			IF_ERR_THEN_RETURN;

			//���̶�ȡ��ǰ������Ϣ
			ret = CalibraReadADCResult(ADCResult);
			IF_ERR_THEN_RETURN;
			
			//���̳ɹ��������ĵ��Ƶ����ĵ��Ҳ�
			if (*ADCResult >ADCResultCenter)
			{
				*GainCalibraFlag = TRUE;
				break;
			}
		}	
	}
	else
	{
		//�ο���ѹֵ�������
		LpcdADCRefernce = ADC_REFERNCE_MAX;
		ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)<<5)+LpcdBiasCurrent&0x7);
		IF_ERR_THEN_RETURN;
		ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
		IF_ERR_THEN_RETURN;

		//���̶�ȡ��ǰ������Ϣ
		ret = CalibraReadADCResult(ADCResult);
		IF_ERR_THEN_RETURN;
		
		//���̳ɹ���־��ʼ��
		*GainCalibraFlag = TRUE;
		
		//�ж��Ƿ����̫С�����̫Сlpcd_gain�Ŵ�
		if (*ADCResult > ADCResultCenter)
		{
			//������Ҫ����
			*GainCalibraFlag = FALSE;
			while(1)
			{
				//�����ǰ�Ѿ���������棬����ʧ��
				if (LpcdGainAmplify == 0x7)
				{
					*GainCalibraFlag = FALSE;
					break;
				}
				else//�����Ŵ�
				{
					LpcdGainAmplify++;  
				}
				//��������
				ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
				IF_ERR_THEN_RETURN;

				//���̶�ȡ��ǰ������Ϣ
				ret = CalibraReadADCResult(ADCResult);
				IF_ERR_THEN_RETURN;
				
				//���̳ɹ��������ĵ��Ƶ����ĵ����
				if (*ADCResult < ADCResultCenter)
				{
					*GainCalibraFlag = TRUE;
					break;
				}
			}
		}
	}
	return TRUE;
}

//***********************************************
//�������ƣ�LpcdInitCalibra()
//�������ܣ���ʼ������
//��ڲ�����uchar *CalibraFlag ���̱�־���Ƿ���Ҫ����
//���ڲ�����uchar  TRUE�����̳ɹ�   FALSE:����ʧ��
//***********************************************
unsigned char  LpcdInitCalibra(unsigned char *CalibraFlag)
{
	unsigned char ret;
	unsigned char ADCResult;				//LPCD������Ϣ
	unsigned char GainCalibraFlag;			//������̽��


	//��������
	ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
	IF_ERR_THEN_RETURN;

	//����ƫ�õ����Ͳο���ѹ
	ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)>>1)+LpcdBiasCurrent&0x7);
	IF_ERR_THEN_RETURN;

	//V02 ���� CalibVmidEnʱ��
	ret = SetReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN);     //������Уģʽ��Vmindʹ��
	IF_ERR_THEN_RETURN;

	//Timer1Cfg����
	ret = SetReg_Ext(JREG_LPCD_T1CFG,(T3ClkDivK<<4)+Timer1Cfg);
	IF_ERR_THEN_RETURN;

	//Timer2Cfg����
	ret = SetReg_Ext(JREG_LPCD_T2CFG,Timer2Cfg);
	IF_ERR_THEN_RETURN;

	//Timer3Cfg����
	ret = SetReg_Ext(JREG_LPCD_T3CFG,Timer3Cfg);
	IF_ERR_THEN_RETURN;
	
/*************************************************************************************************
****
****        ��У����PGA�������˥�������趨
**************************************************************************************************/
  ret = LpcdSet_PGA(&GainCalibraFlag,&ADCResult);                     //PGA��������
	IF_ERR_THEN_RETURN;
	
	//����������ʧ�ܣ���ʧ��
	if (GainCalibraFlag == FALSE)
	{		
		(*CalibraFlag) = FALSE;
		return ADCResult;	   //����ʧ�ܷ��ط���
	}
	//ɨ��ο���ѹֵ���ҵ����ʵĴ�Խ���ĵ������
	(*CalibraFlag) = FALSE;
	
/*************************************************************************************************
****
****        ��У����ADC�Ĳο���ѹ�趨
**************************************************************************************************/
	GainCalibraFlag = LpcdSet_ADCRefvoltage(CalibraFlag,&ADCResult);    //���ú��ʵ�ADC�ο���ѹ
	
/*************************************************************************************************
****
****        ��У���̼���������趨
**************************************************************************************************/	
	ret = LpcdSet_DetectSensitive(LPCD_DetectSensitive);
	IF_ERR_THEN_RETURN;
	
	if (GainCalibraFlag == FALSE)
	{		
		(*CalibraFlag) = FALSE;
		//���� ���̽����ر�CalibVmidEn
		ret = ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN,0);
		IF_ERR_THEN_RETURN;
		return ADCResult;	   //����ʧ�ܷ��ط���
	}
	
	//���� ���̽����ر�CalibVmidEn
	ret = ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN,0);
	IF_ERR_THEN_RETURN;
	return TRUE;
}

//***********************************************
//�������ƣ�WaitForLpcdIrq()
//�������ܣ��ȴ�LPCD�ж�
//��ڲ�����IrqType
//���ڲ�����uchar  TRUE����ȡ�ɹ�   FALSE:ʧ��
//***********************************************
unsigned char WaitForLpcdIrq(unsigned char IrqType)
{
	unsigned char ExtRegData;
	unsigned char ret;
	unsigned char TimeOutCount;
	
	TimeOutCount = 0;
	ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
	//debugo
	if (ret == 0)
	{
		ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
		ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
		ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
		ret =1;
	}
	IF_ERR_THEN_RETURN;
	while ((ExtRegData & IrqType) != IrqType)
	{
		ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
			//debug
		if (ret == 0)
		{
			ret =1;
		}
		IF_ERR_THEN_RETURN;
		//��ʱ�˳�
		delay_ms(10);	//��ʱ10ms
		TimeOutCount++;
		if  (TimeOutCount > IRQ_TIMEOUT)  
		{
			return FALSE; //150ms ��ʱ�˳�
		}
	}
	return TRUE; 
	
}

/*
0: 	no irq
1�� CardDetIrq
2:	calibIrq
3:  AwupIrq
*/
unsigned char GetIrqSource(void)
{
	unsigned char ExtRegData;
	unsigned char ret;
	
	ret = GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);
	if(ret==FALSE)
		return 0;
	else
	{
		if(ExtRegData & JBIT_CARD_IN_IRQ)
			return 1;
		else if(ExtRegData & JBIT_CALIB_IRQ)
			return 2;
		else if(ExtRegData & JBIT_AUTO_WUP_IRQ)
			return 3;
		else 
			return 0;
	}
		
}



//***********************************************
//�������ƣ�ReadLpcdADCResult()
//�������ܣ� ��ȡLPCD����
//��ڲ�����uchar *ADCResult
//���ڲ�����uchar  TRUE����ȡ�ɹ�   FALSE:ʧ��
//***********************************************
unsigned char ReadLpcdADCResult(unsigned char *ADCResult)
{
	unsigned char ExtRegData;
	unsigned char ret;
	unsigned char ADCResultTemp;

	//��ȡ�����Ϣ
	*ADCResult = 0;
	
	ret = GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	ret = GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
   
	 *ADCResult = ADCResultTemp;

	

	//��λLPCD�Ĵ���
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	//��λ�ſ�LPCD�Ĵ���
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;

	return TRUE;
}


//***********************************************
//�������ƣ�CalibraReadADCResult()
//�������ܣ� ���̲���ȡLPCD����
//��ڲ�����uchar *ADCResult
//���ڲ�����uchar  TRUE����ȡ�ɹ�   FALSE:ʧ��
//***********************************************
unsigned char CalibraReadADCResult(unsigned char *ADCResult)
{
	//ʹ�ܵ���ģʽ
	unsigned char ret;
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_CALIBRA_EN);
	IF_ERR_THEN_RETURN;
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_CALIBRA_EN);
	IF_ERR_THEN_RETURN;
	delay_ms(10);	
	//�ȴ����̽����ж�
	ret = WaitForLpcdIrq(JBIT_CALIB_IRQ);
	//debug
	if (ret == 0)
	{
		ret =1;
	}
	IF_ERR_THEN_RETURN;
	//�رյ���ģʽ
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_CALIBRA_EN);
	IF_ERR_THEN_RETURN;
	//��ȡ������Ϣ
	ret = ReadLpcdADCResult(ADCResult);
	IF_ERR_THEN_RETURN;
	return TRUE;
}

//***********************************************
//�������ƣ�LpcdCardIn_IRQHandler()
//�������ܣ���Ƭ�����������
//��ڲ�����
//���ڲ�����
//***********************************************
void LpcdCardIn_IRQHandler(void)
{
	unsigned char ExtRegData;
	unsigned char ADCResultTemp;

    	
	//��ȡ�����Ϣ
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
	//��λLPCD�Ĵ���
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	//��λ�ſ�LPCD�Ĵ���
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	//��Ϊÿ�λ��ѣ����ԼĴ����ᱻ��λ
	//LpcdAuxSelect(ON);					
	//��Ҫ��ʱ�������п������һ���Ĵ���д����
	delay_us(100*1000);
	//-----------------------------------------------
	//OLED��ʾ
	//-----------------------------------------------

	//-----------------------------------------------
	
	//-----------------------------------------------
	//�û��ڴ�����жϿ�Ƭ����֮��Ĵ�������request��
	//Ҳ���Ը��ݿ�Ƭ�Ƿ���ʵ���������µ����Զ���У����
	//-----------------------------------------------
	
}
//***********************************************
//�������ƣ�LpcdAutoWakeUp_IRQHandler()
//�������ܣ��Զ������жϷ������
//��ڲ�������
//���ڲ�������
//***********************************************
void LpcdAutoWakeUp_IRQHandler(void)
{
	
	uchar CalibraFlag;
	unsigned char ExtRegData;
	unsigned char ADCResultTemp;

	//��ȡ�����Ϣ
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
    CalibraFlag = LPCD_IRQ_int();
    
	//LpcdParamInit();									//LPCD������ʼ��
	//LpcdRegisterInit();       							//LPCD�Ĵ�����ʼ��
	//LpcdAuxSelect(ON);									//����AUX�۲�ͨ��
	//LpcdInitCalibra(&CalibraFlag);						//LPCD��ʼ������

	if (CalibraFlag == TRUE)							//������̳ɹ���������
	{
		GPIO_SetBits(GPIOA, RFID_BACKLIGHT);
		delay_us(1000*1000);
		GPIO_ResetBits(GPIOA, RFID_BACKLIGHT);
		GPIO_ResetBits(GPIOA, MF_VCCCTRL);									//����LPCDģʽ
	} 
	else
		GPIO_SetBits(GPIOA, RFID_BACKLIGHT);                    //ʧ�������˸һ��

}


uint8_t LPCD_IRQ_int(void)
{
    uint8_t CalibraFlag;
    LpcdParamInit();									//LPCD������ʼ��
    LpcdRegisterInit();       							//LPCD�Ĵ�����ʼ��
    LpcdInitCalibra(&CalibraFlag);						//LPCD��ʼ������  
    return CalibraFlag;
}
//***********************************************
//�������ƣ�LpcdSetPGA_GainReduce()
//�������ܣ����ú��ʵ�PGA����˥��
//��ڲ�����unsigned char *ADCResult ���ȼ��ֵ�����ַ
//���ڲ�����uchar  TRUE�����óɹ�  FALSE:����ʧ��
//***********************************************
/*unsigned char  LpcdSetPGA_GainReduce(unsigned char *ADCResult)
{
	unsigned char ret;
	while(1)
		{
			//�����ǰ�Ѿ�����С���棬����ʧ��
			if (LpcdGainReduce == 0)
			{
				return FALSE;
			}
			//����˥��
			LpcdGainReduce --; 
			 
			//��������
			ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
			IF_ERR_THEN_RETURN;

			//���̶�ȡ��ǰ������Ϣ
			ret = CalibraReadADCResult(ADCResult);
			IF_ERR_THEN_RETURN;
			
			//���̳ɹ��������ĵ��Ƶ����ĵ��Ҳ�
			if (*ADCResult >ADCResultCenter)
			{
				return TRUE;
			}
		}	
}*/

//***********************************************
//�������ƣ�LpcdSetPGA_GainAmplify()
//�������ܣ����ú��ʵ�PGA����˥��
//��ڲ�����unsigned char *ADCResult ���ȼ��ֵ�����ַ
//���ڲ�����uchar  TRUE�����óɹ�  FALSE:����ʧ��
//***********************************************
/*unsigned char  LpcdSetPGA_GainAmplify(unsigned char *ADCResult)
{
  unsigned char ret;
	while(1)
			{
				//�����ǰ�Ѿ���������棬����ʧ��
				if (LpcdGainAmplify == 0x7)
				{
					return FALSE;
				}
				else//�����Ŵ�
				{
					LpcdGainAmplify++;  
				}
				//��������
				ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
				IF_ERR_THEN_RETURN;

				//���̶�ȡ��ǰ������Ϣ
				ret = CalibraReadADCResult(ADCResult);
				IF_ERR_THEN_RETURN;
				
				//���̳ɹ��������ĵ��Ƶ����ĵ����
				if (*ADCResult < ADCResultCenter)
				{
					return TRUE;
				}
			}
}*/


