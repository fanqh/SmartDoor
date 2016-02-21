#include "74HC595.h"
#include "delay.h"
#include "seg_led.h"
#include "led_blink.h"

#define HC595_LDO_CTRL_PIN                  GPIO_Pin_13									
#define HC595_LDO_CTRL_PORT              		GPIOC
#define HC595_LDO_CTRL_PORT_ENABLE()       	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE)


#define LED_DATA_PIN                  GPIO_Pin_4									
#define LED_DATA_PORT              		GPIOA

#define LED_EN_PIN                    GPIO_Pin_5									
#define LED_EN_PORT              			GPIOA

#define LED_LATCH_PIN                 GPIO_Pin_6								
#define LED_LATCH_PORT              	GPIOA

#define LED_CLK_PIN                   GPIO_Pin_7							
#define LED_CLK_PORT              		GPIOA

#define LED_PORT_ENABLE()       RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
//DATA operation
#define LED_DATA_LOW()						GPIO_ResetBits(LED_DATA_PORT, LED_DATA_PIN)
#define LED_DATA_HIGH()						GPIO_SetBits(LED_DATA_PORT, LED_DATA_PIN)
//CLK operation
#define LED_CLK_LOW()							GPIO_ResetBits(LED_CLK_PORT, LED_CLK_PIN)
#define LED_CLK_HIGH()						GPIO_SetBits(LED_CLK_PORT, LED_CLK_PIN)
//LATCH operation
#define LED_LATCH_LOW()						GPIO_ResetBits(LED_LATCH_PORT, LED_LATCH_PIN)
#define LED_LATCH_HIGH()					GPIO_SetBits(LED_LATCH_PORT, LED_LATCH_PIN)
//EN  operation
#define LED_ENABLE()							GPIO_ResetBits(LED_EN_PORT, LED_EN_PIN)
#define LED_DISABLE()							GPIO_SetBits(LED_EN_PORT, LED_EN_PIN)



#define DOT_PORT_ENABLE()       RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
//pin define
#define DOT_DATA_PIN                  GPIO_Pin_15	
#define DOT_EN_PIN                    GPIO_Pin_12									
#define DOT_LATCH_PIN                 GPIO_Pin_14								
#define DOT_CLK_PIN                   GPIO_Pin_13		

//DATA operation
#define DOT_DATA_LOW()						GPIO_ResetBits(GPIOB, DOT_DATA_PIN)
#define DOT_DATA_HIGH()						GPIO_SetBits(GPIOB, DOT_DATA_PIN)
//CLK operation
#define DOT_CLK_LOW()							GPIO_ResetBits(GPIOB, DOT_CLK_PIN)
#define DOT_CLK_HIGH()						GPIO_SetBits(GPIOB, DOT_CLK_PIN)
//LATCH operation
#define DOT_LATCH_LOW()						GPIO_ResetBits(GPIOB, DOT_LATCH_PIN)
#define DOT_LATCH_HIGH()					GPIO_SetBits(GPIOB, DOT_LATCH_PIN)
//EN  operation
#define DOT_ENABLE()							GPIO_ResetBits(GPIOB, DOT_EN_PIN)
#define DOT_DISABLE()							GPIO_SetBits(GPIOB, DOT_EN_PIN)

typedef void (*fun_io)(uint8_t state);
typedef void (*init)(void);
	

void dot_data_io(uint8_t state)
{
	if(state)
			DOT_DATA_HIGH();
	else
		DOT_DATA_LOW();
}

void dot_clk_io(uint8_t state)
{
	if(state)
		DOT_CLK_HIGH();
	else
		DOT_CLK_LOW();
}

void dot_latch_io(uint8_t state)
{
	if(state)
		DOT_LATCH_HIGH();
	else
		DOT_LATCH_LOW();
}
void dot_en_io(uint8_t state)
{
	if(state)
		DOT_ENABLE();	
	else
		DOT_DISABLE();
}


void led_data_io(uint8_t state)
{
	if(state)
			LED_DATA_HIGH();
	else
		LED_DATA_LOW();
}

void led_clk_io(uint8_t state)
{
	if(state)
		LED_CLK_HIGH();
	else
		LED_CLK_LOW();
}

void led_latch_io(uint8_t state)
{
	if(state)
		LED_LATCH_HIGH();
	else
		LED_LATCH_LOW();
}
void led_en_io(uint8_t state)
{
	if(state)
		LED_ENABLE();	
	else
		LED_DISABLE();
}

struct HC595_OPERRATION
{
	fun_io data_io;
	fun_io clk_io;
	fun_io latch_io;
	fun_io en_io;
};

struct HC595_OPERRATION hc595_led_operation =
{
	led_data_io,	
	led_clk_io,
	led_latch_io,
	led_en_io,
};

struct HC595_OPERRATION hc595_dot_operation =
{
	dot_data_io,	
	dot_clk_io,
	dot_latch_io,
	dot_en_io,
};



void Vcc_Ctrl_LDO_init(void)
{
		GPIO_InitTypeDef GPIO_InitStruct;
	
	
	GPIO_InitStruct.GPIO_Pin = HC595_LDO_CTRL_PIN;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
	GPIO_Init(HC595_LDO_CTRL_PORT, &GPIO_InitStruct);	
}

void HC595_init(uint8_t number)
{
	GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;		        
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
	
	Vcc_Ctrl_LDO_init();
	HC595_Power_OFF();
	if(number & SER_DOT_INTERFACE)
	{
		DOT_PORT_ENABLE();
		GPIO_InitStruct.GPIO_Pin = (DOT_DATA_PIN | DOT_EN_PIN | DOT_LATCH_PIN | DOT_CLK_PIN);		           
		GPIO_Init(GPIOB, &GPIO_InitStruct);	
		DOT_CLK_LOW();
		DOT_DATA_LOW();
		DOT_ENABLE();//enable chip

	}
	if(number & SER_LED_INTERFACE)
	{
		LED_PORT_ENABLE();
		GPIO_InitStruct.GPIO_Pin = (LED_DATA_PIN | LED_EN_PIN | LED_LATCH_PIN | LED_CLK_PIN);		           
		GPIO_Init(GPIOA, &GPIO_InitStruct);	//端口没有定义宏
		LED_ENABLE();//enable chip
	}
	
	HC595_ShiftOut16(SER_LED_INTERFACE, LED_ALL_OFF_VALUE);
//	HC595_ShiftOut16(SER_DOT_INTERFACE, 0xffff);
	HC595_Updata(SER_DOT_INTERFACE, 00);
	HC595_Power_ON();
	
	
	
}


bool HC595_ShiftOut(uint8_t num, uint8_t value)
{
	int8_t i;
	struct HC595_OPERRATION *ops;
	
	if(num==SER_DOT_INTERFACE)
		ops = &hc595_dot_operation;
	else if(num==SER_LED_INTERFACE)
	 ops = &hc595_led_operation;
	else 
		return false;
		
	for(i=7; i>=0; i--)
	{
	
		if(value & (1<<i))	//first bit will prensent Q7
			ops->data_io(1);
		else
			ops->data_io(0);
		
		delay_us(1);
		ops->clk_io(0);
		delay_us(1);
		ops->clk_io(1);

	}
//	ops->clk_io(0);
	
	return true;
}


//display only 0-99
bool HC595_ShiftOut16(uint8_t num,uint16_t code)
{
	
	if(!((num!=SER_DOT_INTERFACE)||(num!=SER_LED_INTERFACE)))
		return false;
	
	HC595_Latch(num, 0);
	HC595_ShiftOut(num, (uint8_t) code&0xff);
	HC595_ShiftOut(num, (uint8_t)((code>>8)&0xff));
//	HC595_ShiftOut(num, 0x00);
	HC595_Latch(num, 1);
	
	return true;
}

//high 8 bit was zero 
bool HC595_ShiftOut24(uint8_t num,uint16_t code)
{
	
	if(!((num!=SER_DOT_INTERFACE)||(num!=SER_LED_INTERFACE)))
		return false;
	
	HC595_Latch(num, 0);
	HC595_ShiftOut(num, (uint8_t) code&0xff);
	HC595_ShiftOut(num, (uint8_t)((code>>8)&0xff));
	HC595_ShiftOut(num, 0x00);
	HC595_Latch(num, 1);
	
	return true;
}

bool HC595_Updata(uint8_t num, uint8_t value)
{
	struct HC595_OPERRATION *ops;
	
	if(num==SER_DOT_INTERFACE)
		ops = &hc595_dot_operation;
	else if(num==SER_LED_INTERFACE)
	  ops = &hc595_led_operation;
	else 
		return false;
	
	ops->latch_io(0);
	Hal_SEG_LED_Output_PreState();
	HC595_ShiftOut(num, value);
	ops->latch_io(1);
	
	return true;
}
bool HC595_Latch(uint8_t num, uint8_t state)
{
	struct HC595_OPERRATION *ops;
	
	if(num==SER_DOT_INTERFACE)
		ops = &hc595_dot_operation;
	else if(num==SER_LED_INTERFACE)
	  ops = &hc595_led_operation;
	else 
		return false;
	if(state)
		ops->latch_io(1);
	else
		ops->latch_io(0);
	return true;
}


void HC595_Power_ON(void)
{
		GPIO_ResetBits(HC595_LDO_CTRL_PORT, HC595_LDO_CTRL_PIN);
}

void HC595_Power_OFF(void)
{
		GPIO_SetBits(HC595_LDO_CTRL_PORT, HC595_LDO_CTRL_PIN);
}




