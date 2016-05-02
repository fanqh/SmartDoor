/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/
//<<< Use Configuration Wizard   in Context Menu >>>
// <h> LPCDʱ������
//     <o0.0..3> LPCD������߽׶�ʱ�����ã��û����ݵ͹���Ҫ�󣬿��޸ģ� 		
//									<0=>0s
//									<1=>300ms	
//									<2=>400ms 
//									<3=>500ms(Default)
//									<4=>600ms
//									<5=>700ms 
//									<6=>800ms
//									<7=>900ms
//									<8=>1.0s
//									<9=>1.1s
//									<10=>1.2s
//									<11=>1.3s
//								   	<12=>1.4s
//									<13=>1.5s
//									<14=>1.6s
//									<15=>1.7s



//     <o1.0..4> LPCD���׶�ʱ������ ���û����ݵ͹���Ҫ�󣬿��޸ģ�
//									<0=>0us
//									<1=>0us
//									<2=>4.7us						
//									<3=>9.4us	
//									<4=>14.1us 
//									<5=>18.8us
//									<6=>23.5us
//									<7=>28.2us 
//									<8=>32.9us
//									<9=>37.6us
//									<10=>42.3us
//									<11=>47us
//									<12=>51.7us(Default)
//									<13=>56.4us
//								   	<14=>61.1us
//									<15=>65.8us
//									<16=>70.5us
//									<17=>75.2us
//									<18=>79.9us
//									<19=>84.6us
//									<20=>89.3us
//									<21=>94us
//									<22=>98.7us
//									<23=>103.4us
//									<24=>108.1us
//									<25=>112.8us
//									<26=>117.5us
//									<27=>122.2us
//									<28=>126.9us
//									<29=>131.6us
//									<30=>136.3us
//									<31=>141us

// </h>




// <h> LPCD������������
//     <e2>    LPCD�Ƿ�ʹ��TX2����(Defaultʹ��)
//	   </e>	 

//     <o3.0..1> LPCD����NMOS�������ã��û����޸ģ�
//									<0=>46ŷķ
//									<1=>23ŷķ(Default)

//     <o4.0..2> LPCD����PMOS�������ã��û����޸ģ�
//									<0x0=>180ŷķ
//									<0x1=>90ŷķ
//									<0x2=>46ŷķ
//									<0x3=>23ŷķ(Default)
//									<0x4=>12ŷķ 
//									<0x5=>6ŷķ
//									<0x6=>3ŷķ
//									<0x7=>1.5ŷķ
// </h>

// <h> LPCD����������
//     <o5.0..5> LPCD������������������ 		
//									<2=>25% 	
//									<3=>12.5%	
//									<4=>6.25%(Default)
//									<5=>3.125% 

//     <o6.0> LPCD�Զ���������������ж� 
//									<0=>����
//									<1=>һ��(Default)

//     <e7>    LPCD�Ƿ�����ж�(Defaultʹ��)
//	   </e>



//     <e8> LPCDʹ���Զ�����ģʽ ���û����޸ģ�
//	   </e>

//     <o9.0..2> LPCD�����Զ�����ʱ�� ���û����޸ģ�
//									<0=>6��
//									<1=>12��
//									<2=>15����(Default)
//									<3=>30����
//									<4=>1Сʱ
//									<5=>1.8Сʱ
//									<6=>3.6Сʱ
//									<7=>7.2Сʱ
// </h>



#define  	TIMER1_CFG  	  			3			//0
#define  	TIMER3_CFG  	  			11			//1
#define  	LPCD_TX2RFEN				1			//2
#define		LPCD_CWN					0x1			
#define		LPCD_CWP					0x4	//4
#define		LPCD_THRESHOLD_RATIO		4			//5
#define   LPCD_DetectSensitive          0.14 //%5--------������Խ���ܹ��������ľ���ԽС��������ΧӦ��4%~15%֮�䣩0.14
#define		LPCD_AUTO_DETECT_TIMES		1			//6
#define  	LPCD_IE 					0x01		//7
#define   LPCD_DS           0x00
#define		AUTO_WUP_EN					1			//
#define		AUTO_WUP_CFG				6			//0(6��) 1(12��) 2(15����) 3(30����) 4(1Сʱ)  5(1.8Сʱ)  6(3.6Сʱ)  7(7.2Сʱ)

//<<< end of configuration section >>>
#include "RF1356.h"  


