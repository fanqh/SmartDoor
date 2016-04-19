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
/* 	FM175XX LPCD API函数库											 */
/* 	主要功能:														 */
/* 		1.实现LPCD相关API函数										 */
/* 	编制:罗挺松 													 */
/* 	编制时间:2014年7月18日											 */
/* 																	 */
/*********************************************************************/

#include "lpcd_api.h"
#include "delay.h" 

#define IF_ERR_THEN_RETURN if (ret == FALSE) return FALSE
#define ADC_REFERNCE_MIN 0x0	//实际并联电容值最小
#define ADC_REFERNCE_MAX 0x7F	//实际电容值最大

typedef  unsigned char uchar;  
//********************************************************************
//内部函数列表
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
//调教变量列表
//********************************************************************
unsigned char T3ClkDivK ;
unsigned char LpcdBiasCurrent ;//3bit  //由样片测试指标决定，在config文件中设定
unsigned char LpcdGainReduce;//2bit
unsigned char LpcdGainAmplify;//3bit
unsigned char LpcdADCRefernce;//7bit 

unsigned char Timer1Cfg;//4bit
unsigned char Timer2Cfg;//5bit
unsigned char Timer3Cfg;//5bit

unsigned char ADCResultFullScale;	//T3下满幅ADCResult信息
unsigned char ADCResultThreshold;	//检测幅度，设置成相对值
unsigned char LpcdThreshold_L;		//LPCD幅度低阈值
unsigned char LpcdThreshold_H;		//LPCD幅度高阈值
unsigned char ADCResultCenter;		//LPCD幅度中心点
unsigned char LpcdADCResult[10];	//Lpcd幅度信息，用于误触发判断

//***********************************************
//函数名称：LpcdParamInit()
//函数功能：LPCD参数初始化
//入口参数：
//出口参数：uchar  TRUE：读取成功   FALSE:失败
//***********************************************
unsigned char LpcdParamInit()
{
	LpcdBiasCurrent = LPCD_BIAS_CURRENT ; //由样片测试指标决定，在config文件中设定
 	LpcdGainReduce = 0x3;			//1x
 	LpcdGainAmplify = 0x0;			//1x
	LpcdADCRefernce = 0;
	Timer1Cfg = TIMER1_CFG;
	Timer2Cfg = TIMER2_CFG;
  Timer3Cfg = TIMER3_CFG;
	
	if (Timer3Cfg > 0xF) //Timer3Cfg用到5bit，所以只能选择16分频
	{
		T3ClkDivK = 2;			//16分频
		ADCResultFullScale =  ((Timer3Cfg - 1)<<3);
		ADCResultCenter = (ADCResultFullScale >>1);
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);
	}
	else if(Timer3Cfg > 0x7) //Timer3Cfg用到4bit，所以只能选择8分频
	{
		T3ClkDivK = 1;			//8分频
		ADCResultFullScale =  ((Timer3Cfg - 1)<<4);          //160
		ADCResultCenter = (ADCResultFullScale >>1);          //80
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);     //10
	}
	else 
	{
		T3ClkDivK = 0;			//4分频
		ADCResultFullScale =  ((Timer3Cfg - 1)<<5);
		ADCResultCenter = (ADCResultFullScale >>1);
		ADCResultThreshold = (ADCResultFullScale >> LPCD_THRESHOLD_RATIO);
	}

	LpcdThreshold_H = ADCResultCenter + ADCResultThreshold;    //80+10=90
	LpcdThreshold_L= ADCResultCenter - ADCResultThreshold;     //80-10=70

	return TRUE;
}
//***********************************************
//函数名称：LpcdRegisterInit()
//函数功能：LPCD寄存器初始化
//入口参数：
//出口参数：uchar  TRUE：初始化成功   FALSE:初始化失败
//***********************************************
#define  COMMIEN_DEF          0x80
#define  DIVIEN_DEF           0x80
#define ComIEnReg	0x02
#define DivIEnReg	0x03
unsigned char LpcdRegisterInit(void)
{
	unsigned char ret;
	unsigned char regdata;
	
	//中断设置
	regdata = COMMIEN_DEF;			 //中断设置

	RF1356_MasterWriteData(ComIEnReg,regdata);

	regdata = DIVIEN_DEF;			 
	RF1356_MasterWriteData(DivIEnReg,regdata);//配置IRQ引脚作为标准CMOS输出，覵Tatus1Reg的IRQ位相反

	//-----------------------------------------------------------------
	//LpcdCtr1寄存器
	//-----------------------------------------------------------------
	//复位LPCD寄存器
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	//复位放开LPCD寄存器
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	
	//使能LPCD功能
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_EN);
	IF_ERR_THEN_RETURN;
	
	//配置LPCD中断
	ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_IE<<5)+JBIT_LPCD_IE);      //配置LPCD中断寄存器状态反映到IRQ引脚
	IF_ERR_THEN_RETURN;	
	
	//配置进场检测次数
	ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_IE<<5)+JBIT_LPCD_CMP_1);       //一次检测到卡有效
	//ret = SetReg_Ext(JREG_LPCD_CTRL1,(LPCD_DS<<5)+JBIT_LPCD_CMP_3);       //3次检测到卡有效
	IF_ERR_THEN_RETURN;

	
	//-----------------------------------------------------------------
	//LpcdCtrl2寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_CTRL2,((LPCD_TX2RFEN<<4)+(LPCD_CWN<<3)+LPCD_CWP));//P管驱动能力从0到7一次增大，这里选了3
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdCtrl3寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_CTRL3,LPCD_MODE<<3);        //没看出实际意义
	IF_ERR_THEN_RETURN;

	//-----------------------------------------------------------------
	//Timer1Cfg寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T1CFG,(T3ClkDivK<<4)+Timer1Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//Timer2Cfg寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T2CFG,Timer2Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//Timer3Cfg寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_T3CFG,Timer3Cfg);
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//VmidBdCfg寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_VMIDBD_CFG,VMID_BG_CFG);    //不建议用户修改
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//AutoWupCfg寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(AUTO_WUP_EN<<3)+AUTO_WUP_CFG);         //设置自动唤醒时间
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_L1寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(LpcdThreshold_L & 0x3F));
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_L2寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(LpcdThreshold_L>>6));         //设置卡检测下阀值
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_H1寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(LpcdThreshold_H& 0x3F));
	IF_ERR_THEN_RETURN;
	//-----------------------------------------------------------------
	//LpcdThreshold_H2寄存器
	//-----------------------------------------------------------------
	ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_H,(LpcdThreshold_H>>6));        //设置卡检测上阀值
	IF_ERR_THEN_RETURN;
	

	//-----------------------------------------------------------------
	//Auto_Wup_Cfg寄存器
	//-----------------------------------------------------------------
	ret=SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(AUTO_WUP_EN<<3) + AUTO_WUP_CFG ); //再次设置自动唤醒时间
	IF_ERR_THEN_RETURN;
		
	return TRUE;
}

//***********************************************
//函数名称：LpcdSet_DetectSensitive()
//函数功能：设置检测灵敏度
//入口参数loat Sensitive：灵敏度
//出口参数：无
//***********************************************
unsigned char  LpcdSet_DetectSensitive(float Sensitive)
{
	  unsigned char ret,Threshold;
	   Threshold =ADCResultCenter*Sensitive; 
	//微调阈值
			/****LpcdThreshold_H = ADCResultCenter + ADCResultThreshold;*/
			/****LpcdThreshold_L= ADCResultCenter - ADCResultThreshold;*/
			LpcdThreshold_H = ADCResultCenter + Threshold;
			LpcdThreshold_L = ADCResultCenter - Threshold;
			//-----------------------------------------------------------------
			//LpcdThreshold_L1寄存器
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(LpcdThreshold_L& 0x3F));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_L2寄存器
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(LpcdThreshold_L>>6));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_H1寄存器
			//-----------------------------------------------------------------
			ret = SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(LpcdThreshold_H& 0x3F));
			IF_ERR_THEN_RETURN;
			//-----------------------------------------------------------------
			//LpcdThreshold_H2寄存器
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
//函数名称：LpcdSet_ADCRefvoltage()
//函数功能：设置合适的ADC参考电压
//入口参数nsigned char *CalibraFlag, unsigned char *ADCResult   幅度检测结果保存地址
//出口参数：uchar  TRUE：设置成功  FALSE:设置失败
//***********************************************
unsigned char  LpcdSet_ADCRefvoltage(unsigned char *CalibraFlag, unsigned char *ADCResult)
{
	  unsigned char ret;
		unsigned char ADCResult_Pre;			//幅度信息的前一个值
	
	//扫描充电电容
	for(LpcdADCRefernce = ADC_REFERNCE_MIN;LpcdADCRefernce < ADC_REFERNCE_MAX;LpcdADCRefernce++)
	//通过此循环可以确定最终的参考电压和ADC中心比较值
	{
		//配置参考电压值
		ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)<<5)+LpcdBiasCurrent&0x7);
		IF_ERR_THEN_RETURN;
		ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
		IF_ERR_THEN_RETURN;

		//备份幅度信息
		ADCResult_Pre = *ADCResult;
		//调教读取当前幅度信息
		ret = CalibraReadADCResult(ADCResult);
		IF_ERR_THEN_RETURN;
		//==============================================
		//算法一
		//==============================================
		//如果幅度开始比中心值小，因为开始比中心值大
		if (*ADCResult < ADCResultCenter)
		{
			//调教成功
			(*CalibraFlag) = TRUE;
			//与前一个调教参考电压，判断哪个幅度更接近中心点
			if((ADCResultCenter - *ADCResult) < (ADCResult_Pre-ADCResultCenter))
			{
				//直接用当前值作为中心点
				ADCResultCenter = *ADCResult;
			}
			else
			{
				//直接用当前值作为中心点
				ADCResultCenter = ADCResult_Pre;
				//参考电压采用之前的参考电压
				LpcdADCRefernce--;
				//重新配置参考电压值
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
//函数名称：LpcdSet_PGA()
//函数功能：设置合适的PGA参数
//入口参数nsigned char *CalibraFlag, unsigned char *ADCResult   幅度检测结果保存地址
//出口参数：uchar  TRUE：设置成功  FALSE:设置失败
//***********************************************
unsigned char  LpcdSet_PGA(unsigned char *GainCalibraFlag, unsigned char *ADCResult)
{
	unsigned char ret;
	//参考电压值配置最小
	LpcdADCRefernce = ADC_REFERNCE_MIN;
	ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)>>1)+LpcdBiasCurrent&0x7);
	IF_ERR_THEN_RETURN;
	ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
	IF_ERR_THEN_RETURN;

	//调教读取当前幅度信息
	ret = CalibraReadADCResult(ADCResult);
	IF_ERR_THEN_RETURN;

	//缺省增益不需要调教
	*GainCalibraFlag = TRUE;

	//判断是否幅度太窄，如果太窄lpcd_gain衰减
	if  (*ADCResult < ADCResultCenter)
	{
		//增益需要调教
		*GainCalibraFlag = FALSE;
		//*GainCalibraFlag = LpcdSetPGA_GainReduce(ADCResult);   //设置PGA增益衰减
		while(1)
		{
			//如果当前已经是最小增益，调教失败
			if (LpcdGainReduce == 0)
			{
				*GainCalibraFlag = FALSE;
				break;
			}
			//继续衰减
			LpcdGainReduce --; 
			 
			//配置增益
			ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
			IF_ERR_THEN_RETURN;

			//调教读取当前幅度信息
			ret = CalibraReadADCResult(ADCResult);
			IF_ERR_THEN_RETURN;
			
			//调教成功，把中心点移到中心点右侧
			if (*ADCResult >ADCResultCenter)
			{
				*GainCalibraFlag = TRUE;
				break;
			}
		}	
	}
	else
	{
		//参考电压值配置最大
		LpcdADCRefernce = ADC_REFERNCE_MAX;
		ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)<<5)+LpcdBiasCurrent&0x7);
		IF_ERR_THEN_RETURN;
		ret = SetReg_Ext(JREG_LPCD_ADC_REFERECE,LpcdADCRefernce&0x3F);
		IF_ERR_THEN_RETURN;

		//调教读取当前幅度信息
		ret = CalibraReadADCResult(ADCResult);
		IF_ERR_THEN_RETURN;
		
		//调教成功标志初始化
		*GainCalibraFlag = TRUE;
		
		//判断是否幅度太小，如果太小lpcd_gain放大
		if (*ADCResult > ADCResultCenter)
		{
			//增益需要调教
			*GainCalibraFlag = FALSE;
			while(1)
			{
				//如果当前已经是最大增益，调教失败
				if (LpcdGainAmplify == 0x7)
				{
					*GainCalibraFlag = FALSE;
					break;
				}
				else//继续放大
				{
					LpcdGainAmplify++;  
				}
				//配置增益
				ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
				IF_ERR_THEN_RETURN;

				//调教读取当前幅度信息
				ret = CalibraReadADCResult(ADCResult);
				IF_ERR_THEN_RETURN;
				
				//调教成功，把中心点移到中心点左侧
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
//函数名称：LpcdInitCalibra()
//函数功能：初始化调教
//入口参数：uchar *CalibraFlag 调教标志，是否需要调教
//出口参数：uchar  TRUE：调教成功   FALSE:调教失败
//***********************************************
unsigned char  LpcdInitCalibra(unsigned char *CalibraFlag)
{
	unsigned char ret;
	unsigned char ADCResult;				//LPCD幅度信息
	unsigned char GainCalibraFlag;			//增益调教结果


	//配置增益
	ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
	IF_ERR_THEN_RETURN;

	//配置偏置电流和参考电压
	ret = SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((LpcdADCRefernce&0x40)>>1)+LpcdBiasCurrent&0x7);
	IF_ERR_THEN_RETURN;

	//V02 增加 CalibVmidEn时能
	ret = SetReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN);     //开启调校模式中Vmind使能
	IF_ERR_THEN_RETURN;

	//Timer1Cfg配置
	ret = SetReg_Ext(JREG_LPCD_T1CFG,(T3ClkDivK<<4)+Timer1Cfg);
	IF_ERR_THEN_RETURN;

	//Timer2Cfg配置
	ret = SetReg_Ext(JREG_LPCD_T2CFG,Timer2Cfg);
	IF_ERR_THEN_RETURN;

	//Timer3Cfg配置
	ret = SetReg_Ext(JREG_LPCD_T3CFG,Timer3Cfg);
	IF_ERR_THEN_RETURN;
	
/*************************************************************************************************
****
****        调校过程PGA的增益和衰减参数设定
**************************************************************************************************/
  ret = LpcdSet_PGA(&GainCalibraFlag,&ADCResult);                     //PGA参数设置
	IF_ERR_THEN_RETURN;
	
	//如果增益调教失败，则失败
	if (GainCalibraFlag == FALSE)
	{		
		(*CalibraFlag) = FALSE;
		return ADCResult;	   //调教失败返回幅度
	}
	//扫描参考电压值，找到合适的穿越中心点的配置
	(*CalibraFlag) = FALSE;
	
/*************************************************************************************************
****
****        调校过程ADC的参考电压设定
**************************************************************************************************/
	GainCalibraFlag = LpcdSet_ADCRefvoltage(CalibraFlag,&ADCResult);    //设置合适的ADC参考电压
	
/*************************************************************************************************
****
****        调校过程检测灵敏度设定
**************************************************************************************************/	
	ret = LpcdSet_DetectSensitive(LPCD_DetectSensitive);
	IF_ERR_THEN_RETURN;
	
	if (GainCalibraFlag == FALSE)
	{		
		(*CalibraFlag) = FALSE;
		//增加 调教结束关闭CalibVmidEn
		ret = ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN,0);
		IF_ERR_THEN_RETURN;
		return ADCResult;	   //调教失败返回幅度
	}
	
	//增加 调教结束关闭CalibVmidEn
	ret = ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIB_VMID_EN,0);
	IF_ERR_THEN_RETURN;
	return TRUE;
}

//***********************************************
//函数名称：WaitForLpcdIrq()
//函数功能：等待LPCD中断
//入口参数：IrqType
//出口参数：uchar  TRUE：读取成功   FALSE:失败
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
		//超时退出
		delay_ms(10);	//延时10ms
		TimeOutCount++;
		if  (TimeOutCount > IRQ_TIMEOUT)  
		{
			return FALSE; //150ms 超时退出
		}
	}
	return TRUE; 
	
}

/*
0: 	no irq
1： CardDetIrq
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
//函数名称：ReadLpcdADCResult()
//函数功能： 读取LPCD幅度
//入口参数：uchar *ADCResult
//出口参数：uchar  TRUE：读取成功   FALSE:失败
//***********************************************
unsigned char ReadLpcdADCResult(unsigned char *ADCResult)
{
	unsigned char ExtRegData;
	unsigned char ret;
	unsigned char ADCResultTemp;

	//读取宽度信息
	*ADCResult = 0;
	
	ret = GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	ret = GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
   
	 *ADCResult = ADCResultTemp;

	

	//复位LPCD寄存器
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;
	//复位放开LPCD寄存器
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	IF_ERR_THEN_RETURN;

	return TRUE;
}


//***********************************************
//函数名称：CalibraReadADCResult()
//函数功能： 调教并读取LPCD幅度
//入口参数：uchar *ADCResult
//出口参数：uchar  TRUE：读取成功   FALSE:失败
//***********************************************
unsigned char CalibraReadADCResult(unsigned char *ADCResult)
{
	//使能调教模式
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
	//等待调教结束中断
	ret = WaitForLpcdIrq(JBIT_CALIB_IRQ);
	//debug
	if (ret == 0)
	{
		ret =1;
	}
	IF_ERR_THEN_RETURN;
	//关闭调教模式
	ret = SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_CALIBRA_EN);
	IF_ERR_THEN_RETURN;
	//读取幅度信息
	ret = ReadLpcdADCResult(ADCResult);
	IF_ERR_THEN_RETURN;
	return TRUE;
}

//***********************************************
//函数名称：LpcdCardIn_IRQHandler()
//函数功能：卡片进场服务程序
//入口参数：
//出口参数：
//***********************************************
void LpcdCardIn_IRQHandler(void)
{
	unsigned char ExtRegData;
	unsigned char ADCResultTemp;

    	
	//读取宽度信息
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
	//复位LPCD寄存器
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR+JBIT_LPCD_RSTN);
	//复位放开LPCD寄存器
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET+JBIT_LPCD_RSTN);
	//因为每次唤醒，测试寄存器会被复位
	//LpcdAuxSelect(ON);					
	//需要延时，否则有可能最后一个寄存器写不进
	delay_us(100*1000);
	//-----------------------------------------------
	//OLED显示
	//-----------------------------------------------

	//-----------------------------------------------
	
	//-----------------------------------------------
	//用户在此添加判断卡片进场之后的处理，比如request等
	//也可以根据卡片是否真实进场，重新调用自动调校程序
	//-----------------------------------------------
	
}
//***********************************************
//函数名称：LpcdAutoWakeUp_IRQHandler()
//函数功能：自动唤醒中断服务程序
//入口参数：无
//出口参数：无
//***********************************************
void LpcdAutoWakeUp_IRQHandler(void)
{
	
	uchar CalibraFlag;
	unsigned char ExtRegData;
	unsigned char ADCResultTemp;

	//读取宽度信息
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	ADCResultTemp = (ExtRegData & 0x3) << 6;
	
	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);
	//IF_ERR_THEN_RETURN;
	
	ADCResultTemp += (ExtRegData&0x3F) ;//& 0x3F)
    CalibraFlag = LPCD_IRQ_int();
    
	//LpcdParamInit();									//LPCD参数初始化
	//LpcdRegisterInit();       							//LPCD寄存器初始化
	//LpcdAuxSelect(ON);									//开启AUX观测通道
	//LpcdInitCalibra(&CalibraFlag);						//LPCD初始化调教

	if (CalibraFlag == TRUE)							//如果调教成功，则亮灯
	{
		GPIO_SetBits(GPIOA, RFID_BACKLIGHT);
		delay_us(1000*1000);
		GPIO_ResetBits(GPIOA, RFID_BACKLIGHT);
		GPIO_ResetBits(GPIOA, MF_VCCCTRL);									//进入LPCD模式
	} 
	else
		GPIO_SetBits(GPIOA, RFID_BACKLIGHT);                    //失败则灯闪烁一次

}


uint8_t LPCD_IRQ_int(void)
{
    uint8_t CalibraFlag;
    LpcdParamInit();									//LPCD参数初始化
    LpcdRegisterInit();       							//LPCD寄存器初始化
    LpcdInitCalibra(&CalibraFlag);						//LPCD初始化调教  
    return CalibraFlag;
}
//***********************************************
//函数名称：LpcdSetPGA_GainReduce()
//函数功能：设置合适的PGA增益衰减
//入口参数：unsigned char *ADCResult 幅度检测值保存地址
//出口参数：uchar  TRUE：设置成功  FALSE:设置失败
//***********************************************
/*unsigned char  LpcdSetPGA_GainReduce(unsigned char *ADCResult)
{
	unsigned char ret;
	while(1)
		{
			//如果当前已经是最小增益，调教失败
			if (LpcdGainReduce == 0)
			{
				return FALSE;
			}
			//继续衰减
			LpcdGainReduce --; 
			 
			//配置增益
			ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
			IF_ERR_THEN_RETURN;

			//调教读取当前幅度信息
			ret = CalibraReadADCResult(ADCResult);
			IF_ERR_THEN_RETURN;
			
			//调教成功，把中心点移到中心点右侧
			if (*ADCResult >ADCResultCenter)
			{
				return TRUE;
			}
		}	
}*/

//***********************************************
//函数名称：LpcdSetPGA_GainAmplify()
//函数功能：设置合适的PGA增益衰减
//入口参数：unsigned char *ADCResult 幅度检测值保存地址
//出口参数：uchar  TRUE：设置成功  FALSE:设置失败
//***********************************************
/*unsigned char  LpcdSetPGA_GainAmplify(unsigned char *ADCResult)
{
  unsigned char ret;
	while(1)
			{
				//如果当前已经是最大增益，调教失败
				if (LpcdGainAmplify == 0x7)
				{
					return FALSE;
				}
				else//继续放大
				{
					LpcdGainAmplify++;  
				}
				//配置增益
				ret = SetReg_Ext(JREG_LPCD_CTRL4,((LpcdGainAmplify << 2) + LpcdGainReduce));
				IF_ERR_THEN_RETURN;

				//调教读取当前幅度信息
				ret = CalibraReadADCResult(ADCResult);
				IF_ERR_THEN_RETURN;
				
				//调教成功，把中心点移到中心点左侧
				if (*ADCResult < ADCResultCenter)
				{
					return TRUE;
				}
			}
}*/


