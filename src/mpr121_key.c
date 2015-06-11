//#include "link_list.h"
#include "iic.h"
#include "mpr121_key.h"
#include "Link_list.h"
#include "fifo.h"
#include "event.h"
#include "pwm.h"

#define MHD_R	    0x2B
#define NHD_R	    0x2C
#define	NCL_R 	    0x2D
#define	FDL_R	    0x2E
#define	MHD_F	    0x2F
#define	NHD_F	    0x30
#define	NCL_F	    0x31
#define	FDL_F	    0x32
#define	ELE0_T	    0x41
#define	ELE0_R	    0x42
#define	ELE1_T	    0x43
#define	ELE1_R	    0x44
#define	ELE2_T	    0x45
#define	ELE2_R	    0x46
#define	ELE3_T	    0x47
#define	ELE3_R	    0x48
#define	ELE4_T	    0x49
#define	ELE4_R	    0x4A
#define	ELE5_T	    0x4B
#define	ELE5_R	    0x4C
#define	ELE6_T	    0x4D
#define	ELE6_R	    0x4E
#define	ELE7_T	    0x4F
#define	ELE7_R	    0x50
#define	ELE8_T  	0x51
#define	ELE8_R	    0x52
#define	ELE9_T	    0x53
#define	ELE9_R	    0x54
#define	ELE10_T	    0x55
#define	ELE10_R	    0x56
#define	ELE11_T	    0x57
#define	ELE11_R	    0x58
#define	FIL_CFG	    0x5D
#define	ELE_CFG	    0x5E
#define GPIO_CTRL0	0x73
#define	GPIO_CTRL1	0x74
#define GPIO_DATA	0x75
#define	GPIO_DIR	0x76
#define	GPIO_EN		0x77
#define	GPIO_SET	0x78
#define	GPIO_CLEAR	0x79
#define	GPIO_TOGGLE	0x7A
#define	ATO_CFG0	0x7B
#define	ATO_CFGU    0x7D
#define	ATO_CFGL	0x7E
#define	ATO_CFGT	0x7F
#define SOFT_RESET  0x80
// Global Constants
#define TOU_THRESH 	0x0B
#define	REL_THRESH  0x07

#define MPR121_ADDR     0xB4

#define MAX_KEY_NUM   12

#define MPR121_IRQ_PIN      GPIO_Pin_2
#define IIC_ByteWrite       I2C_Write
uint8_t I2C_ReadB(uint8_t    iAddress)
{
	uint8_t temp;
	
	if(IICread(MPR121_ADDR, iAddress, &temp, 1)==true)
		return temp;
	else
	{
		IIC_Stop(); 
		return 0;
	}
}

bool I2C_Write(uint8_t iAddress, uint8_t dat)
{  
	bool ret = false;
	
	ret = IICwrite(MPR121_ADDR, iAddress, &dat, 1);	
	if(ret ==false)
		IIC_Stop(); 
	return ret;
  
}

uint8_t touch_key_buf[20];
fifo_struct touch_key_fifo;
struct node_struct_t touch_key_ns;

#if  FAN
void clr_touch_key_buf(void)
{
    fifo_clear(&touch_key_fifo);
}

int16_t acquire_touch_key(void)  
{
    return fifo_acquire(&touch_key_fifo);
}

int16_t get_touch_key(uint8_t *pucKey,uint32_t time_out)
{
    uint32_t  tv=time_out;
    int16_t   result;

    while(1)
    {
        result=fifo_out(&touch_key_fifo,pucKey);
        if(result==0)
            return TA_SUCCESS;
        if(time_out)
        {
            tv--;
            if(tv==0)
                return TA_TIME_OUT;
        }
        delay_us(1*1000);
        if(fifo_acquire(&uart_data_fifo)>=7)
        {
            *pucKey=JATA_KEY_FGP_BREAK;
            return TA_BREAK;
        }
    }
}

#if 0
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'3','6','9',
'#','0','8',
'5','2','*',
'7','4','1'
};
#else
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'*','7','4',
'1','2','5',
'8','0','3',
'6','9','#'
};

	// 1 2 4 5 6 7 9 10
#endif

uint16_t uwKeyStatus[MAX_KEY_NUM];
uint16_t uwTouchBits=0;
uint16_t uwBackCloseDelay=2;              // ????5X20ms???????
uint8_t  ucTouchSwitch=false;             // ?????,??????mpr121_reopen?mpr121_close???
uint8_t  ucKeyPrePress=0;
struct node_struct_t touch_key_ns;

void touch_key_scan(void *priv)         // ??????????KEY??
{
    uint16_t  i,uwBit;
    uint8_t   ucKey=0;

    if(uwBackCloseDelay)
        uwBackCloseDelay--;
    else
        mpr121_backlight_open();
        
    if(mpr121_get_irq_status()==0)
    {
        uwTouchBits=I2C_ReadB(0x00);
        uwTouchBits|=I2C_ReadB(0x01)<<8;   
    }
    
    for(i=0;i<MAX_KEY_NUM;i++)
    {
        uwBit=(uwTouchBits>>i)&0x0001;
        if(uwBit)
        { 
            if(uwKeyStatus[i]<2*TOUCH_LONG_TIME)
            {
                uwKeyStatus[i]++;
            }
            if(uwKeyStatus[i]==TOUCH_LONG_TIME) // ????????????????
            {
                ucKey=ucKeyIndx[i]|LONG_KEY_MASK;
                fifo_in(&touch_key_fifo,ucKey);
            }
            if(uwKeyStatus[i]==TOUCH_SHORT_TIME)
                ucKeyPrePress=ucKeyIndx[i];
        }
        else
        {   
            if(uwKeyStatus[i]>=TOUCH_SHORT_TIME&&uwKeyStatus[i]<TOUCH_LONG_TIME)
            {
                ucKey=ucKeyIndx[i];
                fifo_in(&touch_key_fifo,ucKey);
            }
            uwKeyStatus[i]=0;
        }
    }   
    if(ucKeyPrePress)
    {
        uwBackCloseDelay=2;
        if(ucKeyPrePress=='#')
            uwBackCloseDelay=10;    // '#' ?????,????????,????????????
        mpr121_backlight_close();
        if(ucKeyPrePress>='0'&&ucKeyPrePress<='9')
            Api_Broadcast_Voice_Start(VOICE_KEY);
        ucKeyPrePress=0;
    }
}
void touch_key_hadware_init(void)
{
    mpr121_gpio_init();
}

uint16_t is_touch_wkup(void)
{
    if(mpr121_get_irq_status())
        return false;
    else
        return true;
}

#define TouchThre           5//30//8
#define ReleaThre           3//25//5
#define Prox_TouchThre      5//6      
#define Prox_ReleaThre      0//4
#define STDBY_TCH_THRE      3

#define IIC_ByteWrite       I2C_Write

void mpr121_init_config(void)
{
	
		GPIO_InitTypeDef	GPIO_InitStructure;
	
	
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Pin = MPR121_IRQ_PIN;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    memset(uwKeyStatus,0,sizeof(uwKeyStatus));

    IIC_ByteWrite(0x80,0x63);  //Soft reset
    IIC_ByteWrite(0x5E,0x00);  //Stop mode   

    //touch pad baseline filter
    //rising                                     
    IIC_ByteWrite(0x2B,0x01); //0xFF// MAX HALF DELTA Rising
    IIC_ByteWrite(0x2C,0x01); //0xFF// NOISE HALF DELTA Rising
    IIC_ByteWrite(0x2D,0x05); // //0 NOISE COUNT LIMIT Rising
    IIC_ByteWrite(0x2E,0x00); // DELAY LIMIT Rising
    //falling
    IIC_ByteWrite(0x2F,0x01); // MAX HALF DELTA Falling
    IIC_ByteWrite(0x30,0x01); // NOISE HALF DELTA Falling
    IIC_ByteWrite(0x31,0xFF); // NOISE COUNT LIMIT Falling
    IIC_ByteWrite(0x32,0x02); // //2//DELAY LIMIT Falling
    //touched
    IIC_ByteWrite(0x33,0x00); // Noise half delta touched 
    IIC_ByteWrite(0x34,0x00); // Noise counts touched
    IIC_ByteWrite(0x35,0x00); //Filter delay touched


    //Prox baseline filter
    //rising
    IIC_ByteWrite(0x36,0x0f); //0xFF// MAX HALF DELTA Rising
    IIC_ByteWrite(0x37,0x0f); //0xFF// NOISE HALF DELTA Rising
    IIC_ByteWrite(0x38,0x00); // NOISE COUNT LIMIT Rising
    IIC_ByteWrite(0x39,0x00); // DELAY LIMIT Rising
    //falling
    IIC_ByteWrite(0x3A,0x01); // MAX HALF DELTA Falling
    IIC_ByteWrite(0x3B,0x01); // NOISE HALF DELTA Falling
    IIC_ByteWrite(0x3C,0xff); // NOISE COUNT LIMIT Falling
    IIC_ByteWrite(0x3D,0xff); // DELAY LIMIT Falling
    //touched
    IIC_ByteWrite(0x3E,0x00); // ff 
    IIC_ByteWrite(0x3F,0x00); // 1F    
    IIC_ByteWrite(0x40,0x00); // 


    //Touch pad threshold

    IIC_ByteWrite(0x41,TouchThre); // ELE0 TOUCH THRESHOLD
    IIC_ByteWrite(0x42,ReleaThre); // ELE0 RELEASE THRESHOLD
    IIC_ByteWrite(0x43,TouchThre); // ELE1 TOUCH THRESHOLD
    IIC_ByteWrite(0x44,ReleaThre); // ELE1 RELEASE THRESHOLD
    IIC_ByteWrite(0x45,TouchThre); // ELE2 TOUCH THRESHOLD
    IIC_ByteWrite(0x46,ReleaThre); // ELE2 RELEASE THRESHOLD
    IIC_ByteWrite(0x47,TouchThre); // ELE3 TOUCH THRESHOLD
    IIC_ByteWrite(0x48,ReleaThre); // ELE3 RELEASE THRESHOLD
    IIC_ByteWrite(0x49,TouchThre); // ELE4 TOUCH THRESHOLD
    IIC_ByteWrite(0x4A,ReleaThre); // ELE4 RELEASE THRESHOLD
    IIC_ByteWrite(0x4B,TouchThre); // ELE5 TOUCH THRESHOLD
    IIC_ByteWrite(0x4C,ReleaThre); // ELE5 RELEASE THRESHOLD
    IIC_ByteWrite(0x4D,TouchThre); // ELE6 TOUCH THRESHOLD
    IIC_ByteWrite(0x4E,ReleaThre); // ELE6 RELEASE THRESHOLD
    IIC_ByteWrite(0x4F,TouchThre); // ELE7 TOUCH THRESHOLD
    IIC_ByteWrite(0x50,ReleaThre); // ELE7 RELEASE THRESHOLD
    IIC_ByteWrite(0x51,TouchThre); // ELE8 TOUCH THRESHOLD
    IIC_ByteWrite(0x52,ReleaThre); // ELE8 RELEASE THRESHOLD
    IIC_ByteWrite(0x53,TouchThre); // ELE9 TOUCH THRESHOLD
    IIC_ByteWrite(0x54,ReleaThre); // ELE9 RELEASE THRESHOLD
    IIC_ByteWrite(0x55,TouchThre); // ELE10 TOUCH THRESHOLD
    IIC_ByteWrite(0x56,ReleaThre); // ELE10 RELEASE THRESHOLD
    IIC_ByteWrite(0x57,TouchThre); // ELE11 TOUCH THRESHOLD
    IIC_ByteWrite(0x58,ReleaThre); // ELE11 RELEASE THRESHOLD
      
    //Prox threshold
    IIC_ByteWrite(0x59,Prox_TouchThre); // ELE12 TOUCH THRESHOLD
    IIC_ByteWrite(0x5A,Prox_ReleaThre); // ELE12 RELEASE THRESHOLD

    //touch /release debounce
    IIC_ByteWrite(0x5B,0x00); 

    //AFE configuration
    IIC_ByteWrite(0x5D,0x04);   //SFI=4  X  ESI=16ms    
    IIC_ByteWrite(0x5C,0x80);   //FFI=18  

    //Auto configuration 

    // 08:??????  0B:??????
    IIC_ByteWrite(0x7B,0x8F);  
    IIC_ByteWrite(0x7D,0xE4);  
    IIC_ByteWrite(0x7E,0x94); 
    IIC_ByteWrite(0x7F,0xCD); 
       
    IIC_ByteWrite(0x5E,0xCC);    //????ELE0~ELE4

    fifo_create(&touch_key_fifo,touch_key_buf,sizeof(touch_key_buf));
    lklt_insert(&touch_key_ns,touch_key_scan,NULL,1000/50);
    ucTouchSwitch=true;
}

int16_t mpr121_reopen(void)
{
    if(ucTouchSwitch==false)
        return TA_FAIL;
    __disable_irq();
    uwBackCloseDelay=0;
    __enable_irq();
    mpr121_backlight_open();
    return TA_SUCCESS;
}

int16_t mpr121_close(void)
{
    if(ucTouchSwitch==false)
        return TA_FAIL;
    __disable_irq();
    uwBackCloseDelay=250;
    __enable_irq();
    mpr121_backlight_close();
    return TA_SUCCESS;
}

int16_t mpr121_disable(void)
{
    lklt_del(&touch_key_ns);
    mpr121_close();
    ucTouchSwitch=false;
    return TA_SUCCESS;
}

int16_t mpr121_enter_standby(void)
{
    uint16_t  uwTime=10;

    mpr121_disable();
    
    IIC_ByteWrite(0x5E,0xC0);
    IIC_ByteWrite(0x5D,0x05);    // SFI=4  X  ESI=32ms    

    IIC_ByteWrite(0x41,STDBY_TCH_THRE); // ELE0 TOUCH THRESHOLD
    IIC_ByteWrite(0x43,STDBY_TCH_THRE); // ELE1 TOUCH THRESHOLD
    IIC_ByteWrite(0x45,STDBY_TCH_THRE); // ELE2 TOUCH THRESHOLD
    IIC_ByteWrite(0x47,STDBY_TCH_THRE); // ELE3 TOUCH THRESHOLD
    IIC_ByteWrite(0x49,STDBY_TCH_THRE); // ELE4 TOUCH THRESHOLD
    IIC_ByteWrite(0x4B,STDBY_TCH_THRE); // ELE5 TOUCH THRESHOLD
    IIC_ByteWrite(0x4D,STDBY_TCH_THRE); // ELE6 TOUCH THRESHOLD
    IIC_ByteWrite(0x4F,STDBY_TCH_THRE); // ELE7 TOUCH THRESHOLD
    IIC_ByteWrite(0x51,STDBY_TCH_THRE); // ELE8 TOUCH THRESHOLD
    
    IIC_ByteWrite(0x5E,0xC9);             // 0~8 ELE
    
    while(mpr121_get_irq_status()==0)
    {
        delay_us(10);
        uwTouchBits=I2C_ReadB(0x00);
        uwTouchBits|=I2C_ReadB(0x01)<<8;   
        uwTime--;
        if(uwTime==0)
            break;
    }
    return TA_SUCCESS;
}
#else

#if 1
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'#','9','6',
'1','8','0',
'5','2','3',
'4','7','*'
};
#else
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'*','7','4',
'1','2','5',
'8','0','3',
'6','9','#'
};

	// 1 2 4 5 6 7 9 10
#endif


#define TouchThre           5//30//8
#define ReleaThre           3//25//5
#define Prox_TouchThre      5//6      
#define Prox_ReleaThre      0//4
#define STDBY_TCH_THRE      3

uint16_t uwKeyStatus[MAX_KEY_NUM];
uint16_t uwTouchBits=0;
//uint16_t uwBackCloseDelay=2;              // ????5X20ms???????
//uint8_t  ucTouchSwitch=false;             // ?????,??????mpr121_reopen?mpr121_close???
uint8_t  ucKeyPrePress=0;


void mpr121_init_config(void)
{
	
		GPIO_InitTypeDef	GPIO_InitStructure;
	
	
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Pin = MPR121_IRQ_PIN;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  //  memset(uwKeyStatus,0,sizeof(uwKeyStatus));

    IIC_ByteWrite(0x80,0x63);  //Soft reset
    IIC_ByteWrite(0x5E,0x00);  //Stop mode   

    //touch pad baseline filter
    //rising                                     
    IIC_ByteWrite(0x2B,0x01); //0xFF// MAX HALF DELTA Rising
    IIC_ByteWrite(0x2C,0x01); //0xFF// NOISE HALF DELTA Rising
    IIC_ByteWrite(0x2D,0x05); // //0 NOISE COUNT LIMIT Rising
    IIC_ByteWrite(0x2E,0x00); // DELAY LIMIT Rising
    //falling
    IIC_ByteWrite(0x2F,0x01); // MAX HALF DELTA Falling
    IIC_ByteWrite(0x30,0x01); // NOISE HALF DELTA Falling
    IIC_ByteWrite(0x31,0xFF); // NOISE COUNT LIMIT Falling
    IIC_ByteWrite(0x32,0x02); // //2//DELAY LIMIT Falling
    //touched
    IIC_ByteWrite(0x33,0x00); // Noise half delta touched 
    IIC_ByteWrite(0x34,0x00); // Noise counts touched
    IIC_ByteWrite(0x35,0x00); //Filter delay touched


    //Prox baseline filter
    //rising
    IIC_ByteWrite(0x36,0x0f); //0xFF// MAX HALF DELTA Rising
    IIC_ByteWrite(0x37,0x0f); //0xFF// NOISE HALF DELTA Rising
    IIC_ByteWrite(0x38,0x00); // NOISE COUNT LIMIT Rising
    IIC_ByteWrite(0x39,0x00); // DELAY LIMIT Rising
    //falling
    IIC_ByteWrite(0x3A,0x01); // MAX HALF DELTA Falling
    IIC_ByteWrite(0x3B,0x01); // NOISE HALF DELTA Falling
    IIC_ByteWrite(0x3C,0xff); // NOISE COUNT LIMIT Falling
    IIC_ByteWrite(0x3D,0xff); // DELAY LIMIT Falling
    //touched
    IIC_ByteWrite(0x3E,0x00); // ff 
    IIC_ByteWrite(0x3F,0x00); // 1F    
    IIC_ByteWrite(0x40,0x00); // 


    //Touch pad threshold

    IIC_ByteWrite(0x41,TouchThre); // ELE0 TOUCH THRESHOLD
    IIC_ByteWrite(0x42,ReleaThre); // ELE0 RELEASE THRESHOLD
    IIC_ByteWrite(0x43,TouchThre); // ELE1 TOUCH THRESHOLD
    IIC_ByteWrite(0x44,ReleaThre); // ELE1 RELEASE THRESHOLD
    IIC_ByteWrite(0x45,TouchThre); // ELE2 TOUCH THRESHOLD
    IIC_ByteWrite(0x46,ReleaThre); // ELE2 RELEASE THRESHOLD
    IIC_ByteWrite(0x47,TouchThre); // ELE3 TOUCH THRESHOLD
    IIC_ByteWrite(0x48,ReleaThre); // ELE3 RELEASE THRESHOLD
    IIC_ByteWrite(0x49,TouchThre); // ELE4 TOUCH THRESHOLD
    IIC_ByteWrite(0x4A,ReleaThre); // ELE4 RELEASE THRESHOLD
    IIC_ByteWrite(0x4B,TouchThre); // ELE5 TOUCH THRESHOLD
    IIC_ByteWrite(0x4C,ReleaThre); // ELE5 RELEASE THRESHOLD
    IIC_ByteWrite(0x4D,TouchThre); // ELE6 TOUCH THRESHOLD
    IIC_ByteWrite(0x4E,ReleaThre); // ELE6 RELEASE THRESHOLD
    IIC_ByteWrite(0x4F,TouchThre); // ELE7 TOUCH THRESHOLD
    IIC_ByteWrite(0x50,ReleaThre); // ELE7 RELEASE THRESHOLD
    IIC_ByteWrite(0x51,TouchThre); // ELE8 TOUCH THRESHOLD
    IIC_ByteWrite(0x52,ReleaThre); // ELE8 RELEASE THRESHOLD
    IIC_ByteWrite(0x53,TouchThre); // ELE9 TOUCH THRESHOLD
    IIC_ByteWrite(0x54,ReleaThre); // ELE9 RELEASE THRESHOLD
    IIC_ByteWrite(0x55,TouchThre); // ELE10 TOUCH THRESHOLD
    IIC_ByteWrite(0x56,ReleaThre); // ELE10 RELEASE THRESHOLD
    IIC_ByteWrite(0x57,TouchThre); // ELE11 TOUCH THRESHOLD
    IIC_ByteWrite(0x58,ReleaThre); // ELE11 RELEASE THRESHOLD
      
    //Prox threshold
    IIC_ByteWrite(0x59,Prox_TouchThre); // ELE12 TOUCH THRESHOLD
    IIC_ByteWrite(0x5A,Prox_ReleaThre); // ELE12 RELEASE THRESHOLD

    //touch /release debounce
    IIC_ByteWrite(0x5B,0x00); 

    //AFE configuration
    IIC_ByteWrite(0x5D,0x04);   //SFI=4  X  ESI=16ms    
    IIC_ByteWrite(0x5C,0x80);   //FFI=18  

    //Auto configuration 

    // 08:??????  0B:??????
    IIC_ByteWrite(0x7B,0x8F);  
    IIC_ByteWrite(0x7D,0xE4);  
    IIC_ByteWrite(0x7E,0x94); 
    IIC_ByteWrite(0x7F,0xCD);   
    IIC_ByteWrite(0x5E,0xCC);    //????ELE0~ELE4
		
		fifo_create(&touch_key_fifo,touch_key_buf,sizeof(touch_key_buf));
    lklt_insert(&touch_key_ns,touch_key_scan,NULL,100/2);

}



uint8_t mpr121_get_irq_status(void)
{
    return GPIO_ReadInputDataBit(GPIOB,MPR121_IRQ_PIN);
}



void touch_key_scan(void *priv)         // ??????????KEY??
{
    uint16_t  i,uwBit;
    uint8_t   ucKey=0;
		Hal_EventTypedef evt;
			   
    if(mpr121_get_irq_status()==0)
    {
        uwTouchBits=I2C_ReadB(0x00);
        uwTouchBits|=I2C_ReadB(0x01)<<8;   
    }
    
    for(i=0;i<MAX_KEY_NUM;i++)
    {
        uwBit=(uwTouchBits>>i)&0x0001;
        if(uwBit)
        { 
            if(uwKeyStatus[i]<2*TOUCH_LONG_TIME)
            {
                uwKeyStatus[i]++;
            }
            if(uwKeyStatus[i]==TOUCH_LONG_TIME) // ????????????????
            {
              ucKey=ucKeyIndx[i] | LONG_KEY_MASK;
							evt.event = TOUCH_KEY_EVENT;
							evt.data.KeyValude = ucKey;
							USBH_PutEvent(evt);
							fifo_in(&touch_key_fifo,ucKey & (~LONG_KEY_MASK));
							Hal_Beep_Blink (1, 80, 30);
							printf("long: %c\r\n",ucKey);
            }
            if(uwKeyStatus[i]==TOUCH_SHORT_TIME)
                ucKeyPrePress=ucKeyIndx[i];
        }
        else
        {   
            if(uwKeyStatus[i]>=TOUCH_SHORT_TIME&&uwKeyStatus[i]<TOUCH_LONG_TIME)
            {
                ucKey=ucKeyIndx[i];
								evt.event = TOUCH_KEY_EVENT;
								evt.data.KeyValude = ucKey;
								USBH_PutEvent(evt);
								fifo_in(&touch_key_fifo,ucKey);
								Hal_Beep_Blink (1, 80, 30);
                printf("short: %c\r\n",ucKey);
            }
            uwKeyStatus[i]=0;
        }
    }   
    if(ucKeyPrePress)
    {
        ucKeyPrePress=0;
    }
}


#endif

