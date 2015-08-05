#include<stdio.h>
#include<stdlib.h>
#include "led_blink.h"
#include "74HC595.h"
#include "time.h"
#include "Link_list.h"

#define RED_GREEN_BIT (3<<14)
#define Random(x) (rand() % (x+1)) //get random data
struct node_struct_t led_scan_node;

uint8_t BateryLedFlag=0; //0 表示关，1表示常亮
uint8_t OpenNormallyFlag =0;


HalLedControl_t HalLedControl = {
	0,HAL_LED_MODE_OFF,0,200,200,200,0xffff, LED_BLUE_ALL_ON_VALUE
};

static void Hal_LED_Update (void *priv);


void Led_Battery_Low_ON(void)
{
	BateryLedFlag	= 1;
	HalLedControl.All_Off_Mask &= (~LED_BOTTERY_LOW_WARM_VALUE);
	HC595_ShiftOut16(SER_LED_INTERFACE, (~(uint16_t)LED_BOTTERY_LOW_WARM_VALUE) & LED_ALL_OFF_VALUE);
}

void Led_Battery_Low_OFF(void)
{
//	uint16_t code;
	
	
	BateryLedFlag	= 0;
	HalLedControl.All_Off_Mask |= LED_BOTTERY_LOW_WARM_VALUE;
	HC595_ShiftOut16(SER_LED_INTERFACE, (uint16_t)LED_BOTTERY_LOW_WARM_VALUE | LED_ALL_OFF_VALUE);
}


void Led_Open_Normally_ON(void)
{
	OpenNormallyFlag	= 1;
	HalLedControl.All_Off_Mask &= (~LED_OPEN_NORMALLY_MODE);
	HC595_ShiftOut16(SER_LED_INTERFACE, (~(uint16_t)LED_OPEN_NORMALLY_MODE) & LED_BLUE_ALL_ON_VALUE);
}

void Led_Open_Normally_OFF(void)
{
//	uint16_t code;
	
	
	OpenNormallyFlag	= 0;
	HalLedControl.All_Off_Mask |= LED_OPEN_NORMALLY_MODE;
	HC595_ShiftOut16(SER_LED_INTERFACE, (uint16_t)LED_OPEN_NORMALLY_MODE | LED_ALL_OFF_VALUE);
}


//genrate 0---11 except 4 5 6 7 
static uint8_t RandomSpcData(void)
{
	uint8_t random, m;
	
	random = Random(11);/* radom 0--11*/
#if 0
	if((random>=4) && (random<=7))//passby 4--7
	{
		m = Random(4);//gernate random 0--3
		if(m)
			random = random*m;
		else
			random = random-4;
	}
#endif
	return random;
}

//因为LED为共阳极，所以结果取反  在这里14 15 为红灯和绿灯，应该保持原来的状态
uint16_t Random16bitdata(void)
{
	uint8_t bit;
	uint16_t rad;
	
	bit = RandomSpcData();
	rad = (((((uint16_t) 0x0001)<< bit)) & (~(3<<14)));
	if(BateryLedFlag)
		rad &= (~LED_BOTTERY_LOW_WARM_VALUE);
	else
		rad |= (LED_BOTTERY_LOW_WARM_VALUE);
	
//	printf("bit= %d\r\n", bit);
	return rad;
}


void Hal_LED_Task_Register(void)
{
	 lklt_insert(&led_scan_node, Hal_LED_Update, NULL, 10*TRAV_INTERVAL);//10ms
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
					HalLedControl.DisplayCode = DisplayCode;
					HalLedControl.All_Off_Mask = HalLedControl.All_Off_Mask;  /* all LED turn off*/
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
					HalLedControl.mode  = HAL_LED_MODE_FLASH | HAL_LED_MODE_BLINK;   
					HalLedControl.offtime  = 200;                             
					HalLedControl.ontime = 200;                             
					HalLedControl.left  = 0;      
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
			if(code == LED_RED_ON_VALUE)
				HalLedControl.colour = 1;
			else if(code ==LED_GREEN_ON_VALUE)
				HalLedControl.colour = 1;
			else
				HalLedControl.colour = 0;
			
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
						if(HalLedControl.colour!=0)
							  HC595_ShiftOut16(SER_LED_INTERFACE, LED_ALL_OFF_VALUE);
						else
								HC595_ShiftOut16(SER_LED_INTERFACE, HalLedControl.All_Off_Mask);

						if ( !(HalLedControl.mode & HAL_LED_MODE_FLASH) )
						{
							HalLedControl.left--;                         // Not continuous, reduce count
							if(HalLedControl.left==0)
								HalLedControl.colour = 0;
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
						HC595_ShiftOut16(SER_LED_INTERFACE,HalLedControl.All_Off_Mask);//diaplay off
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

