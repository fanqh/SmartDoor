#include "74HC595.h"
#include "seg_led.h"
#include "Link_list.h"
#include "time.h"


//共阳极SEG8_LED 显示编码
/* 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15  16  17  18  19  20 */
/* 0 1 2 3 4 5 6 7 8 9 A  b  C  d  E  F  ,- , L,  P,  u   n  */
uint8_t LEDDisplayCode[] = {0xc0,0xf9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x00,0x90,0x88,0x83,0xc6,0xA1,0x86,0x8E,   0xBF,0xC7, 0x8C, 0xc8,0xc1};

HalLedControl_t HalSegLedControl = {
	0,HAL_LED_MODE_OFF,0,200,200,200,0xffff
};
struct node_struct_t seg_led_scan_node;
static void Hal_SEG_LED_Update (void *priv);
	
void Hal_SEG_LED_Init(void)
{
	 lklt_insert(&seg_led_scan_node, Hal_SEG_LED_Update, NULL, 1*TRAV_INTERVAL);//2ms
}
	
	
int16_t GetDispalayCodeFromValue(uint8_t value)
{
	int16_t DisplayCode;

	if(value>99)
		return -1;
	
	DisplayCode = LEDDisplayCode[value/10];
	DisplayCode = (DisplayCode<<8) | (value%10);
	
	return DisplayCode;
	
}


#if 0
void Hal_SEG_LED_Display_ON(uint16_t code)
{
	HC595_ShiftOut24(SER_DOT_INTERFACE, HalSegLedControl.DisplayCode);
}

void Hal_SEG_LED_Display_OFF(void)
{
	HC595_ShiftOut24(SER_DOT_INTERFACE,HalSegLedControl.DisplayCode);
}

void Hal_SEG_LED_Blink (uint16_t code, uint32_t numBlinks, uint32_t ontime, uint32_t offtime)
{
			HalSegLedControl.mode  = HAL_SEG_LED_MODE_OFF;                    /*清除之前的状态*/
			HalSegLedControl.offtime  = offtime;                             
			HalSegLedControl.ontime = ontime;                             
			HalSegLedControl.left  = numBlinks;      
			HalSegLedControl.DisplayCode = code;
			if (!numBlinks) HalSegLedControl.mode |= HAL_SEG_LED_MODE_FLASH;  /* 一直闪烁 */
			HalSegLedControl.next = GetSystemTime();  //todo              
			HalSegLedControl.mode |= HAL_SEG_LED_MODE_BLINK;                  
}
void Hal_SEG_LED_Blink_Stop(void)
{
	HalSegLedControl.mode |= HAL_SEG_LED_MODE_BLINK_STOP;
}
#endif

void Hal_SEG_LED_Display_Set(uint8_t mode, uint16_t DisplayCode)/* 显示地位在后 */
{
		switch(mode)
		{
			case HAL_LED_MODE_ON:
					HalSegLedControl.mode = HAL_LED_MODE_ON;
					HalSegLedControl.DisplayCode = DisplayCode;
					HC595_ShiftOut24(SER_DOT_INTERFACE,HalSegLedControl.DisplayCode);
				break;
			case HAL_LED_MODE_OFF:
					HalSegLedControl.mode = HAL_LED_MODE_OFF;
					HalSegLedControl.DisplayCode = 0xffff;  
					HC595_ShiftOut24(SER_DOT_INTERFACE,HalSegLedControl.DisplayCode);
				break;
			case HAL_LED_MODE_BLINK:
					HalSegLedControl.mode  = HAL_LED_MODE_OFF;                    /*清除之前的状态*/
					HalSegLedControl.offtime  = 150;                             
					HalSegLedControl.ontime = 150;                             
					HalSegLedControl.left  = 200;      
					HalSegLedControl.DisplayCode = DisplayCode;
//					if (!numBlinks) HalSegLedControl.mode |= HAL_SEG_LED_MODE_FLASH;  /* 一直闪烁 */
					HalSegLedControl.next = GetSystemTime();  //todo              
					HalSegLedControl.mode |= HAL_LED_MODE_BLINK;  
				break;
			case HAL_LED_MODE_FLASH:
					HalSegLedControl.mode  = HAL_LED_MODE_FLASH | HAL_LED_MODE_BLINK;                   
					HalSegLedControl.offtime  = 150;                             
					HalSegLedControl.ontime = 150;                             
					HalSegLedControl.left  = 0;      
					HalSegLedControl.DisplayCode = DisplayCode;
				break;
			case HAL_LED_MODE_BLINK_STOP:
					HalSegLedControl.mode |= HAL_LED_MODE_BLINK_STOP;
				break;
			default:
				break;
				
		}
}


static void Hal_SEG_LED_Update (void *priv)
{
  uint32_t time;
	
  /* Check if sleep is active or not */
  if (!HalSegLedControl.SleepActive)
  {
			if (HalSegLedControl.mode & HAL_LED_MODE_BLINK)
			{
				time = GetSystemTime();  //todo
				if (time >= HalSegLedControl.next)
				{
					if (HalSegLedControl.mode & HAL_LED_MODE_ON)
					{ 
						HalSegLedControl.next = HalSegLedControl.offtime + time;
						HalSegLedControl.mode &= ~HAL_LED_MODE_ON;        /* not on */
//						HalSegLedControl.DisplayCode = 0xffff;
						HC595_ShiftOut24(SER_DOT_INTERFACE,0xffff);//diaplay off

						if ( !(HalSegLedControl.mode & HAL_LED_MODE_FLASH) )
						{
							HalSegLedControl.left--;                         // Not continuous, reduce count
						}
					}
					else if ( !(HalSegLedControl.left) && !(HalSegLedControl.mode & HAL_LED_MODE_FLASH) )
					{
						HalSegLedControl.mode ^= HAL_LED_MODE_BLINK;       // No more blinks
					}
					else
					{  
						HalSegLedControl.next = HalSegLedControl.ontime + time;
						HalSegLedControl.mode |= HAL_LED_MODE_ON;  
						HC595_ShiftOut24(SER_DOT_INTERFACE,HalSegLedControl.DisplayCode); //display code 
						if(HalSegLedControl.mode & HAL_LED_MODE_BLINK_STOP)
						{
							//HalSegLedControl.mode &= ~HAL_LED_MODE_BLINK;	
							HalSegLedControl.mode = HAL_LED_MODE_ON;
						}
					}
				}
			}
  }
}

void Hal_SEG_LED_Output_PreState(void)
{
	if((HalSegLedControl.mode&HAL_LED_MODE_BLINK) || (HalSegLedControl.mode&HAL_LED_MODE_FLASH))
	{
		if(HalSegLedControl.mode&HAL_LED_MODE_ON)
		{
			HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t) (HalSegLedControl.DisplayCode&0xff));
			HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)((HalSegLedControl.DisplayCode>>8)&0xff));
		}
		else if(!(HalSegLedControl.mode&HAL_LED_MODE_OFF))
		{
			HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)(0xff));
			HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)(0xff));
		}
	}
	else if(HalSegLedControl.mode&HAL_LED_MODE_ON)
	{
		HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t) (HalSegLedControl.DisplayCode&0xff));
		HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)((HalSegLedControl.DisplayCode>>8)&0xff));
	}
	else
	{
		HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)(0xff));
		HC595_ShiftOut(SER_DOT_INTERFACE, (uint8_t)(0xff));
	}
}




