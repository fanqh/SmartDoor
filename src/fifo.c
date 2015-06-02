#include "string.h"
#include "fifo.h"

void fifo_create(fifo_struct *fifo_struct,unsigned char *buff,unsigned short len)
{
	fifo_struct->tt_len=len;
	fifo_struct->size=0;
	fifo_struct->in=fifo_struct->out=0;
	fifo_struct->buff=buff;
}

void fifo_clear(fifo_struct *fifo_struct)
{	//开始工作前缓冲区的初始化工作
	fifo_struct->size=0;
	fifo_struct->in=fifo_struct->out=0;
}

unsigned char fifo_in(fifo_struct *fifo_struct,unsigned char data)
{	//函数不可重入，除非加禁止中断
	if(fifo_struct->size>fifo_struct->tt_len-1)
		return 1;
	if(fifo_struct->in==fifo_struct->tt_len-1)
		{
		*(fifo_struct->buff+fifo_struct->in)=data;
		fifo_struct->in=0;
	}
	else
		{
		*(fifo_struct->buff+fifo_struct->in)=data;
		fifo_struct->in++;
	}
	fifo_struct->size++;
	return 0;
}

unsigned char fifo_out(fifo_struct *fifo_struct,unsigned char *data)
{
	if(!(fifo_struct->size))
		return 1;
	if(fifo_struct->out==fifo_struct->tt_len-1)
		{
		*data=*(fifo_struct->buff+fifo_struct->out);
		fifo_struct->out=0;
	}
	else
		{
		*data=*(fifo_struct->buff+fifo_struct->out);
		fifo_struct->out++;
	}
	fifo_struct->size--;
	return 0;
}

unsigned char fifo_init(fifo_struct *fifo_struct,const char *data,unsigned int len)
{
	if(!data)
		{
		fifo_clear(fifo_struct);
		return 1;
	}
	fifo_struct->in=len;
	fifo_struct->out=0;
	memcpy((void *)fifo_struct->buff,data,len);
	fifo_struct->size=len;
	return 0;
}


