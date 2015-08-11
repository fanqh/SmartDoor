#include "pwm.h"
#include "74HC595.h"//使用头文件中数据结构
#include "time.h"
#include "Link_list.h"
#include "delay.h"

HalLedControl_t HalBeepControl = {
	0,HAL_LED_MODE_OFF,0,200,200,200,0xffff
};
struct node_struct_t Beep_scan_node;//seg_led_scan_node;
static void Hal_Beep_Update (void *priv);

static void Beep_PWM_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;		           
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;		 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_2);//搞了半边，这里忘记打开了，，fuck！
}

static void Beep_PWM_TimeBase_config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
		  /* Set the default configuration */
  TIM_TimeBaseInitStruct.TIM_Period = 450-1;//100-1;//370-1; //300-1
  TIM_TimeBaseInitStruct.TIM_Prescaler =48-1; //480 -1;  //1us
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM16, &TIM_TimeBaseInitStruct);
	
}

static void Beep_PWM_config(void)
{
	TIM_OCInitTypeDef sConfig;
	
		/* Set the default configuration */
	sConfig.TIM_OCMode = TIM_OCMode_PWM1;
	sConfig.TIM_OutputState = TIM_OutputState_Enable;
	sConfig.TIM_OutputNState = TIM_OutputState_Disable;
	sConfig.TIM_Pulse = 130;
	sConfig.TIM_OCPolarity = TIM_OCPolarity_Low;
	sConfig.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OC1Init(TIM16, &sConfig);
	TIM_OC1PreloadConfig(TIM16, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM16, ENABLE );
	TIM_Cmd(TIM16, ENABLE);

}

void Beep_PWM_Init(void)
{
	Beep_PWM_GPIO_Config();
	Beep_PWM_TimeBase_config();
	Beep_PWM_config();
	lklt_insert(&Beep_scan_node, Hal_Beep_Update, NULL, 1*TRAV_INTERVAL);//4ms
}

void Beep_ON(void)
{
	TIM_CtrlPWMOutputs(TIM16, ENABLE);
}
void Beep_OFF(void)
{
	TIM_CtrlPWMOutputs(TIM16, DISABLE);
}

void Beep_Three_Time(void)
{
		Beep_ON();
		delay_ms(20);
		Beep_OFF();
		delay_ms(10);
	
		Beep_ON();
		delay_ms(20);
		Beep_OFF();
		delay_ms(10);
	
		Beep_ON();
		delay_ms(20);
		Beep_OFF();
		
}

void Hal_Beep_Blink (uint32_t numBlinks, uint32_t ontime, uint32_t offtime)
{
			HalBeepControl.mode  = HAL_LED_MODE_OFF;                    /*清除之前的状态*/
			HalBeepControl.offtime  = offtime;                             
			HalBeepControl.ontime = ontime;                             
			HalBeepControl.left  = numBlinks;      
			if (!numBlinks) HalBeepControl.mode |= HAL_LED_MODE_FLASH;  /* 一直闪烁 */
			HalBeepControl.next = GetSystemTime();  //todo              
			HalBeepControl.mode |= HAL_LED_MODE_BLINK;                  
}

static void Hal_Beep_Update (void *priv)
{
  uint32_t time;
	
  /* Check if sleep is active or not */
  if (!HalBeepControl.SleepActive)
  {
			if (HalBeepControl.mode & HAL_LED_MODE_BLINK)
			{
				time = GetSystemTime();  //todo
				if (time >= HalBeepControl.next)
				{
					if (HalBeepControl.mode & HAL_LED_MODE_ON)
					{ 
						HalBeepControl.next = HalBeepControl.offtime + time;
						HalBeepControl.mode &= ~HAL_LED_MODE_ON;        /* not on */
						Beep_OFF();

						if ( !(HalBeepControl.mode & HAL_LED_MODE_FLASH) )
						{
							HalBeepControl.left--;                         // Not continuous, reduce count
						}
					}
					else if ( !(HalBeepControl.left) && !(HalBeepControl.mode & HAL_LED_MODE_FLASH) )
					{
						HalBeepControl.mode ^= HAL_LED_MODE_BLINK;       // No more blinks
					}
					else
					{  
						HalBeepControl.next = HalBeepControl.ontime + time;
						HalBeepControl.mode |= HAL_LED_MODE_ON;  
						Beep_ON(); 
						if(HalBeepControl.mode & HAL_LED_MODE_BLINK_STOP)
						{
							//HalBeepControl.mode &= ~HAL_LED_MODE_BLINK;	
							HalBeepControl.mode = HAL_LED_MODE_ON;
						}
					}
				}
			}
  }
}



