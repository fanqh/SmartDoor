#include "iic.h"
#include "delay.h"

uint8_t ack;
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	I2Cx_SDA_GPIO_CLK_ENABLE();
	
	GPIO_InitStruct.GPIO_Pin = (I2Cx_SCL_PIN | I2Cx_SDA_PIN);		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

}

//----??IIC????-------------------------------------------------------------
void IIC_Start(void)
{
	IIC_SDA_PIN_H();
	IIC_SCL_PIN_H();
	delay_us(10); //9
	IIC_SDA_PIN_L();				//START:when CLK is high,DATA change form high to low
	delay_us(10); //	9
	IIC_SCL_PIN_L();						  //??I2C??,?????????
}
//----??IIC????-------------------------------------------------------------
void IIC_Stop(void)
{
	IIC_SCL_PIN_L();
	IIC_SDA_PIN_L();					//STOP:when CLK is high DATA change form low to high
	delay_us(10);	//9
	IIC_SCL_PIN_H();
	delay_us(10);	//9
	IIC_SDA_PIN_H();	//??I2C??????
	delay_us(10);	//9
}
//----??SCL??----------------------------------------------------
uint8_t IIC_Clock()
{
	uint8_t sample;

	delay_us(10);   //9
	IIC_SCL_PIN_H();      		//??????,????????????
	delay_us(10);		//???????????4µs   9
	sample = IIC_SDA();					  //changed
	IIC_SCL_PIN_L();
	return sample;
}
//----??????----------------------------------------------------
void IIC_SendByte(uint8_t c)
{
	uint8_t BitCnt;
	uint8_t t;
	//?????????8?,
	for (BitCnt = 0; BitCnt < 8; BitCnt++)
	{
		t = (c & 0x80) >> 7;
		if (t == 0)
			IIC_SDA_PIN_L();
		else
			IIC_SDA_PIN_H();

		c <<= 1;
		IIC_Clock();
	}
	if (IIC_Clock())
		ack = 0;
	else
		ack = 1;
}
//----??????----------------------------------------------------
uint8_t IIC_RcvByte(void)
{
	uint8_t retc;
	uint8_t BitCnt;

	retc = 0;
	for (BitCnt = 0; BitCnt < 8; BitCnt++)
	{
		retc = retc << 1;
		if (IIC_Clock())
			retc++;
	}
	return retc;
}
//----??----------------------------------------------------------
void IIC_Ack(uint8_t a)
{
	if (a == 0)
		IIC_SDA_PIN_L();
	else
		IIC_SDA_PIN_H();
	IIC_Clock();
	IIC_SDA_PIN_H();
}

///////////////////////////////////////////////////////////////////
//----?????----------------------------------------------------
bool IICputc(uint8_t sla, uint8_t c)
{
	IIC_Start();          //????
	IIC_SendByte(sla);		//??????
	if (ack == 0)
		return false;

	IIC_SendByte(c);          //????
	if (ack == 0)
		return false;

	IIC_Stop();           //????
	return true;
}
//----?????----------------------------------------------------
bool IICwrite(uint8_t sla, uint8_t suba, uint8_t *s, uint8_t no)
{
	uint8_t i;

	IIC_Start();          //????
	IIC_SendByte(sla);        //??????
	if (ack == 0)
		return false;

	IIC_SendByte(suba);       //???????
	if (ack == 0)
		return false;

	for (i = 0; i < no; i++)
	{
		IIC_SendByte(*s);      //????
		if (ack == 0)
			return false;
		s++;
	}
	IIC_Stop();           //????
	return true;
}
//----?????----------------------------------------------------
bool IICwriteExt(uint8_t sla, uint8_t *s, uint8_t no)
{
	uint8_t i;

	IIC_Start();          //????
	IIC_SendByte(sla);        //??????
	if (ack == 0)
		return false;

	for (i = 0; i < no; i++)
	{
		IIC_SendByte(*s);      //????
		if (ack == 0)
			return false;
		s++;
	}
	IIC_Stop();           //????
	return true;
}
//----?????----------------------------------------------------
bool IICgetc(uint8_t sla, uint8_t *c)
{
	IIC_Start();          //????
	IIC_SendByte(sla + 1);      //??????
	if (ack == 0)
		return false;

	*c = IIC_RcvByte();         //????
	IIC_Ack(1);           //???,??????,????
	IIC_Stop();           //????
	return true;
}
//----?????----------------------------------------------------
bool IICread(uint8_t sla, uint8_t suba, uint8_t *s, uint8_t no)
{
	uint8_t i;

	IIC_Start();          //????
	IIC_SendByte(sla);        //??????
	if (ack == 0)
		return false;

	IIC_SendByte(suba);       //???????
	if (ack == 0)
		return false;

	IIC_Start();			//??????
	IIC_SendByte(sla + 1);
	if (ack == 0)
		return false;

	for (i = 0; i < no - 1; i++)   //????(no-1)??
	{
		*s = IIC_RcvByte();      	//????
		IIC_Ack(0);        //?????,?????
		s++;
	}
	*s = IIC_RcvByte();        //???no??
	IIC_Ack(1);          //???,??????
	IIC_Stop();          //????
	return true;
}
//----?????----------------------------------------------------
bool IICreadExt(uint8_t sla, uint8_t *s, uint8_t no)
{
	uint8_t i;

	IIC_Start();
	IIC_SendByte(sla + 1);		//R/W???,?1???, ?0 ???
	if (ack == 0)
		return false;

	for (i = 0; i < no - 1; i++)   //????(no-1)???
	{
		*s = IIC_RcvByte();      //????
		IIC_Ack(0);        //????,?????
		s++;
	}
	*s = IIC_RcvByte();        //???no??
	IIC_Ack(1);          //???,??????
	IIC_Stop();          //????
	return true;
}

