#include "stm32f0xx.h"
#include "stdio.h"
#include "fifo.h"
#include "delay.h"
#include "uart.h"

#define RX_DATA_LEN_MAX  32


fifo_struct uart_data_fifo;
unsigned char uart_data_buf[RX_DATA_LEN_MAX];

void uart1_Init(void)
{ 
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructureure;
	NVIC_InitTypeDef NVIC_InitStructure; 

	/* USARTx configured as follow:
	- BaudRate = 115200 baud  
	- Word Length = 8 Bits
	- Stop Bit = 1 Stop Bit
	- Parity = No Parity
	- Hardware flow control disabled (RTS and CTS signals)
	- Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	
	  /* Reset GPIO init structure parameters values */
	GPIO_InitStructureure.GPIO_Pin  = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructureure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructureure.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_InitStructureure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructureure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_1) ;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_1) ;
	
	GPIO_Init(GPIOA, &GPIO_InitStructureure);
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	fifo_create(&uart_data_fifo, uart_data_buf, sizeof(uart_data_buf));
	UsartClrBuf();
}

int fputc(int ch, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);

	/* Loop until transmit data register is empty */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

	return ch;
}

void urart_rec(uint8_t c)
{
	fifo_in(&uart_data_fifo,c);
}
void UsartSend(unsigned char *outptr,unsigned int len)
{
    while(len--)
	{
		USART_SendData(USART1,*outptr++);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}

uint32_t UsartGet(uint8_t *buff, uint32_t len, uint32_t timeout)
{
    uint32_t  tmplen,tv;
    uint8_t   result,data;

    tmplen=0;
    tv=timeout;
    while(tv--)
    {
        while(1)
        {
            __disable_irq();
            result=fifo_out(&uart_data_fifo,&data);
            __enable_irq();
            if(result==1)
                break;
            *buff++=data;
            tmplen++;
            if(tmplen>=len)
                return tmplen;
        }
        delay_us(1000);
    }
    return tmplen;
}

void UsartClrBuf(void)
{
    __disable_irq();
    fifo_clear(&uart_data_fifo);
    __enable_irq();
}
uint16_t GetUartSize(void)
{	
	return Get_fifo_size(&uart_data_fifo);
}









