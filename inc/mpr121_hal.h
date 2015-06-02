#ifndef _MPR121_HAL_H_
#define _MPR121_HAL_H_

#define MPR121_IRQ_PIN      GPIO_Pin_2
#define MPR121_SCL_PIN		GPIO_Pin_1
#define MPR121_SDA_PIN		GPIO_Pin_0

extern const gpiic_func_t mpr121_gpio_config;

void mpr121_gpio_init(void);

#endif

