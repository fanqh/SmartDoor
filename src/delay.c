#include "stm32f0xx.h"


static uint8_t  fac_us=0;//us?????
static uint16_t fac_ms=0;//ms?????
//???????
//SYSTICK??????HCLK???1/8
//SYSCLK:????
void delay_init(void)
{
	uint8_t SYSCLK=48;
	
	SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk ;//processor clck as system clck source
	fac_us=SYSCLK;		    
	fac_ms=(uint16_t)fac_us*1000;
   
}								    
//??nms
//??nms???
//SysTick->LOAD?24????,??,?????:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK???Hz,nms???ms
//?72M???,nms<=1864 
void delay_ms(uint16_t nms)
{	 		  	  
	uint32_t temp;		   
	SysTick->LOAD=(uint32_t)nms*fac_ms-1;//????(SysTick->LOAD?24bit)
	SysTick->VAL =0x00;           //?????
	SysTick->CTRL=0x01 ;          //????  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//??????   
	SysTick->CTRL=0x00;       //?????
	SysTick->VAL =0X00;       //?????	  	    
}   
//??nus
//nus?????us?.		    								   
void delay_us(uint32_t nus)
{		
	uint32_t temp;	    	 
	SysTick->LOAD=nus*fac_us-1; //????	  		 
	SysTick->VAL=0x00;        //?????
	SysTick->CTRL=0x01 ;      //???? 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//??????   
	SysTick->CTRL=0x00;       //?????
	SysTick->VAL =0X00;       //?????	 
}

void delay_s(uint16_t s)
{
	while(s)
	{
		delay_ms(1000/8);
		s--;
	}
}






