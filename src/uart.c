#include "stm32f0xx.h"
#include "stdio.h"

void uart1_Init(void)
{ 
  USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructureure;

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










