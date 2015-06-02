#ifndef __I2C_H
#define __I2C_H
//////////////////////////////////////////////////////////////////////////////////	 

#include "stm32f0xx.h"
#include "stdio.h"
#include "stdbool.h"

#define I2Cx_SDA_GPIO_CLK_ENABLE()       RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
#define I2Cx_SCL_GPIO_CLK_ENABLE()       RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);

//#define I2Cx_FORCE_RESET()               __I2C2_FORCE_RESET()
//#define I2Cx_RELEASE_RESET()             __I2C2_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_Pin_1									
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_Pin_0
#define I2Cx_SDA_GPIO_PORT              GPIOB


#define IIC_SDA_PIN_L()					GPIO_ResetBits(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN)
#define IIC_SDA_PIN_H()					GPIO_SetBits(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN)

#define IIC_SCL_PIN_L()					GPIO_ResetBits(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN)
#define IIC_SCL_PIN_H()					GPIO_SetBits(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN)

#define IIC_SDA()               GPIO_ReadInputDataBit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN)


extern uint8_t ack;
bool IICwrite(uint8_t sla, uint8_t suba, uint8_t *s, uint8_t no);
bool IICwriteExt(uint8_t sla, uint8_t *s, uint8_t no);
bool IICgetc(uint8_t sla, uint8_t *c);
bool IICread(uint8_t sla, uint8_t suba, uint8_t *s, uint8_t no);
bool IICreadExt(uint8_t sla, uint8_t *s, uint8_t no);


void IIC_Start(void);
void IIC_Stop(void);
//uint8_t IIC_Clock(void);
void IIC_SendByte(uint8_t c);
uint8_t IIC_RcvByte(void);
//void IIC_Ack(uint8_t a);
//bool IICputc(uint8_t sla, uint8_t c);

void IIC_Init(void);
//////////////////////////////////////////////////////////////////////////////////	 
#endif

