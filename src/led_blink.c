#include<stdio.h>
#include<stdlib.h>
#include "led_blink.h"
#include "74HC595.h"
#include "time.h"
#include "Link_list.h"

#define Random(x) (rand() % (x+1)) //get random data
struct node_struct_t led_scan_node;

HalLedControl_t HalLedControl = {
	0,HAL_LED_MODE_OFF,0,200,200,200,0xffff
};

static void Hal_LED_Update (void *priv);
//genrate 0---14 except 4 5 6 7 
static uint8_t RandomSpcData(void)
{
	uint8_t random, m;
	
	random = Random(15);/* radom 0--15*/
	if((random>=4) && (random<=7))//passby 4--7
	{
		m = Random(4);//gernate random 0--3
		if(m)
			random = random*m;
		else
			random = random-4;
	}
	return random;
}

//因为LED为共阳极，所以结果取反
uint16_t Random16bitdata(void)
{
	uint8_t bit;
	
	bit = RandomSpcData();
	return (~(((uint16_t) 0x0001)<< bit));
}


void Hal_LED_Task_Register(void)
{
	 lklt_insert(&led_scan_node, Hal_LED_Update, NULL, 10/5);
		Hal_LED_Display_Set(HAL_LED_MODE_OFF, LED_ALL_OFF_VALUE);
}

#if 1
void Hal_LED_Display_Set(uint8_t mode, uint16_t DisplayCode)
{
		switch(mode)
		{
			case HAL_LED_MODE_ON:
					HalLedControl.mode = HAL_LED_MODE_ON;
					HalLedControl.DisplayCode = DisplayCode;
					HC595_ShiftOut16(SER_LED_INTERFACE,HalLedControl.DisplayCode);
				break;
			case HAL_LED_MODE_OFF:
					HalLedControl.mode = HAL_LED_MODE_OFF;
					HalLedControl.DisplayCode = LED_ALL_OFF_VALUE;  /* all LED turn off*/
					HC595_ShiftOut16(SER_LED_INTERFACE,HalLedControl.DisplayCode);
				break;
			case HAL_LED_MODE_BLINK:
					HalLedControl.mode  = HAL_LED_MODE_OFF;          /*clear pre state*/
					HalLedControl.offtime  = 200;                             
					HalLedControl.ontime = 200;                             
					HalLedControl.left  = 2;      
					HalLedControl.DisplayCode = DisplayCode;
//					if (!numBlinks) HalLedControl.mode |= HAL_LED_MODE_FLASH;  /* 一直闪烁 */
					HalLedControl.next = GetSystemTime();  //todo              
					HalLedControl.mode |= HAL_LED_MODE_BLINK;  
				break;
			case HAL_LED_MODE_FLASH:
					HalLedControl.mode  = HAL_LED_MODE_FLASH;                    /*清除之前的状态*/
					HalLedControl.offtime  = 200;                             
					HalLedControl.ontime = 200;                             
					HalLedControl.left  = 2;      
					HalLedControl.DisplayCode = DisplayCode;
				break;
			case HAL_LED_MODE_BLINK_STOP:
					HalLedControl.mode |= HAL_LED_MODE_BLINK_STOP;
				break;
			default:
				break;
				
		}
}

void Hal_LED_Blink (uint16_t code, uint32_t numBlinks, uint32_t ontime, uint32_t offtime)
{
			HalLedControl.mode  = HAL_LED_MODE_OFF;                    /*清除之前的状态*/
			HalLedControl.offtime  = offtime;                             
			HalLedControl.ontime = ontime;                             
			HalLedControl.left  = numBlinks;      
			HalLedControl.DisplayCode = code;
			if (!numBlinks) HalLedControl.mode |= HAL_LED_MODE_FLASH;  /* 一直闪烁 */
			HalLedControl.next = GetSystemTime();  //todo              
			HalLedControl.mode |= HAL_LED_MODE_BLINK;                  
}

void Hal_LED_Random_Blink(void)
{
	Hal_LED_Display_Set(HAL_LED_MODE_BLINK, Random16bitdata());

}

void HAL_LED_Blue_ON_Contine(uint16_t code, uint32_t ontime)
{
	HalLedControl.ontime = ontime;  
	HalLedControl.mode  = HAL_LED_MODE_OFF; 
	HalLedControl.DisplayCode = code;
	HalLedControl.next = GetSystemTime();  //todo 
	HalLedControl.mode |= HAL_LED_MODE_ON_CONTINUE;   
}
static void Hal_LED_Update (void *priv)
{
  uint32_t time;
	
  /* Check if sleep is active or not */
  if (!HalLedControl.SleepActive)
  {
			if (HalLedControl.mode & HAL_LED_MODE_BLINK)
			{
				time = GetSystemTime();  //todo
				if (time >= HalLedControl.next)
				{
					if (HalLedControl.mode & HAL_LED_MODE_ON)
					{ 
						HalLedControl.next = HalLedControl.offtime + time;
						HalLedControl.mode &= ~HAL_LED_MODE_ON;        /* not on */
//						HalLedControl.DisplayCode = 0xffff;
						HC595_ShiftOut16(SER_LED_INTERFACE, LED_ALL_OFF_VALUE);//diaplay off

						if ( !(HalLedControl.mode & HAL_LED_MODE_FLASH) )
						{
							HalLedControl.left--;                         // Not continuous, reduce count
						}
					}
					else if ( !(HalLedControl.left) && !(HalLedControl.mode & HAL_LED_MODE_FLASH) )
					{
						HalLedControl.mode ^= HAL_LED_MODE_BLINK;       // No more blinks
					}
					else
					{  
						HalLedControl.next = HalLedControl.ontime + time;
						HalLedControl.mode |= HAL_LED_MODE_ON;  
						HC595_ShiftOut16(SER_LED_INTERFACE,HalLedControl.DisplayCode); //display code 
						if(HalLedControl.mode & HAL_LED_MODE_BLINK_STOP)
						{
							//HalLedControl.mode &= ~HAL_LED_MODE_BLINK;	
							HalLedControl.mode = HAL_LED_MODE_ON;
						}
					}
				}
			}
			if(HalLedControl.mode & HAL_LED_MODE_ON_CONTINUE)
			{
				time = GetSystemTime();
				
				if(time>=HalLedControl.next)
				{
					if (HalLedControl.mode & HAL_LED_MODE_ON)
					{
						HalLedControl.mode &= ~HAL_LED_MODE_ON;        /* not on */
						HalLedControl.mode ^= HAL_LED_MODE_ON_CONTINUE;       // No more blinks
						HC595_ShiftOut16(SER_LED_INTERFACE,LED_ALL_OFF_VALUE);//diaplay off
					}
					else
					{
						HalLedControl.next = HalLedControl.ontime + time;
						HalLedControl.mode |= HAL_LED_MODE_ON; 
						HC595_ShiftOut16(SER_LED_INTERFACE,HalLedControl.DisplayCode); //display code 
					}
				}
			}
		}
}

#endif

