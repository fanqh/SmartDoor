#ifndef __UART_H_
#define __UART_H_
#include "stm32f0xx.h"

extern uint8_t uart_block_flag;

void uart1_Init(void);
void urart_rec(uint8_t c);
void UsartClrBuf(void);
uint32_t UsartGetBlock(uint8_t *buff, uint32_t len, uint32_t timeout);
void UsartSend(unsigned char *outptr,unsigned int len, uint8_t block);
uint16_t GetUartSize(void);
uint16_t GetUartData(uint8_t *buff);

#endif

