#ifndef __UART_H_
#define __UART_H_

void uart1_Init(void);
void urart_rec(uint8_t c);
void UsartClrBuf(void);
uint32_t UsartGet(uint8_t *buff, uint32_t len, uint32_t timeout);
void UsartSend(unsigned char *outptr,unsigned int len);

#endif

