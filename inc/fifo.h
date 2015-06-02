#ifndef _FIFO_H_
#define _FIFO_H_

typedef struct{
	unsigned short tt_len;
	volatile unsigned short size;
	volatile unsigned short in;
	volatile unsigned short out;
	volatile unsigned char *buff;
} fifo_struct;

void fifo_create(fifo_struct *fifo_struct,unsigned char *buff,unsigned short len);
void fifo_clear(fifo_struct *fifo_struct);
unsigned char fifo_in(fifo_struct *,unsigned char);
unsigned char fifo_out(fifo_struct *fifo_struct,unsigned char *data);
unsigned char fifo_init(fifo_struct *fifo_struct,const char *data,unsigned int len);
#endif
