//#include "link_list.h"
#include "iic.h"
#include "mpr121_key.h"
#include "Link_list.h"
#include "fifo.h"
#include "event.h"
#include "pwm.h"
#include "delay.h"
#include "string.h"
#include "process_event.h"
#include "time.h"
#include "button_key.h"

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
#define	ELE8_T  		0x51
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

//#define MAX_KEY_NUM   12

#define MPR121_IRQ_PIN      GPIO_Pin_2
#define IIC_ByteWrite       I2C_Write


struct touch_key_t
{
	uint8_t flag;
	uint32_t timebase;
	uint32_t time;
	uint8_t  ucKeyPrePress;
//	uint8_t ucKeyPrePress;
}uwKeyStatus[MAX_KEY_NUM];

uint8_t factory_mode = 0;
extern uint8_t is_Err_Warm_Flag;

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

uint8_t touch_key_buf[TOUCH_KEY_PSWD_MAX_LEN+1];
fifo_struct touch_key_fifo;
struct node_struct_t touch_key_ns;

#if 0
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'#','9','6',
'3','8','0',
'5','2','1',
'4','7','*'
};
#else
const uint8_t ucKeyIndx[MAX_KEY_NUM]={
'#','7','4',
'1','2','5',
'8','0','3',
'6','9','*'
};

	// 1 2 4 5 6 7 9 10
#endif


#define TouchThre           0x8//5//30//8  v1.0 :5
#define ReleaThre           5//25//5
#define Prox_TouchThre      5//6      
#define Prox_ReleaThre      0//4
#define STDBY_TCH_THRE     4// 0x10//origin 6  

//uint16_t uwKeyStatus[MAX_KEY_NUM];
uint16_t uwTouchBits=0;
//uint8_t  ucKeyPrePress=0;




uint8_t mpr121_get_irq_status(void)
{
	  
    return GPIO_ReadInputDataBit(GPIOB,MPR121_IRQ_PIN);
}


int16_t mpr121_enter_standby(void)
{
    uint16_t  uwTime=10;
	
#if 1
    
    IIC_ByteWrite(0x5E,0xC0);    //original 0xC0
    IIC_ByteWrite(0x5D,0x05);    // SFI=4  X  ESI=32ms    
	IIC_ByteWrite(0x2A,0xff);
	IIC_ByteWrite(0x59,1);            //chen: 0x00 STDBY_TCH_THRE   
	IIC_ByteWrite(0x5A,3);                          
    IIC_ByteWrite(0x5E,0xf0);             //ELE13 proximity enable chen:0xf0
		
#else
    IIC_ByteWrite(0x5E,0xC0);
    IIC_ByteWrite(0x5D,0x00);    //0x05 SFI=4  X  ESI=32ms    

    IIC_ByteWrite(0x41,STDBY_TCH_THRE); // ELE0 TOUCH THRESHOLD
    IIC_ByteWrite(0x43,STDBY_TCH_THRE); // ELE1 TOUCH THRESHOLD
    IIC_ByteWrite(0x45,STDBY_TCH_THRE); // ELE2 TOUCH THRESHOLD
    IIC_ByteWrite(0x47,STDBY_TCH_THRE); // ELE3 TOUCH THRESHOLD
    IIC_ByteWrite(0x49,STDBY_TCH_THRE); // ELE4 TOUCH THRESHOLD
    IIC_ByteWrite(0x4B,STDBY_TCH_THRE); // ELE5 TOUCH THRESHOLD
    IIC_ByteWrite(0x4D,STDBY_TCH_THRE); // ELE6 TOUCH THRESHOLD
    IIC_ByteWrite(0x4F,STDBY_TCH_THRE); // ELE7 TOUCH THRESHOLD
    IIC_ByteWrite(0x51,STDBY_TCH_THRE); // ELE8 TOUCH THRESHOLD
		
	//add by fan*+**************
	IIC_ByteWrite(0x53,STDBY_TCH_THRE); // ELE9 TOUCH THRESHOLD
	IIC_ByteWrite(0x55,STDBY_TCH_THRE); // ELE10 TOUCH THRESHOLD
	IIC_ByteWrite(0x57,STDBY_TCH_THRE); // ELE11 TOUCH THRESHOLD
	 IIC_ByteWrite(0x58,STDBY_TCH_THRE); // ELE11 RELEASE THRESHOLD
    
    IIC_ByteWrite(0x5E,0xCC);             // 0~11 ELE
#endif
    
    while(mpr121_get_irq_status()==0)
    {
        delay_us(10);
        uwTouchBits=I2C_ReadB(0x00);
        uwTouchBits|=I2C_ReadB(0x01)<<8;   
        uwTime--;
        if(uwTime==0)
            break;
    }
    return 1;
}




void mpr121_IRQ_Pin_Config(void)
{
	
	GPIO_InitTypeDef	GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = MPR121_IRQ_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void mpr121_init_config(void)
{
    memset(uwKeyStatus,0,sizeof(struct touch_key_t)*MAX_KEY_NUM);

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

    IIC_ByteWrite(0x41,0x10); // ELE0 TOUCH THRESHOLD
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
    IIC_ByteWrite(0x5D,0x00);   //SFI=4  X  ESI=1ms   //0x4  
    IIC_ByteWrite(0x5C,0x80);   //FFI=18  0

    //Auto configuration 

  // 08:禁止自动配置  0B:使能自动配置
    IIC_ByteWrite(0x7B,0x8F);  
    IIC_ByteWrite(0x7D,0xE4);  
    IIC_ByteWrite(0x7E,0x94); 
    IIC_ByteWrite(0x7F,0xCD); 
	IIC_ByteWrite(0x5E,0xCC);    //????ELE0~ELE4 0xCC
		
	fifo_create(&touch_key_fifo,touch_key_buf,sizeof(touch_key_buf));
//    lklt_insert(&touch_key_ns,touch_key_scan, NULL, 1*2);//2*2ms 执行一次

}





void touch_key_scan(void *priv)         // ??????????KEY??
{
    uint16_t  i,uwBit;
    uint8_t   ucKey=0;
	Hal_EventTypedef evt;
	uint32_t time;
	
	if(is_Err_Warm_Flag==1)
		return;
	
	time = *(uint32_t*)(priv);
    if((mpr121_get_irq_status()==0)&&(GPIO_ReadInputDataBit( KEY_IN_DET_PORT,KEY_IN_DET_PIN)!=0))
    {
        uwTouchBits=I2C_ReadB(0x00);
        uwTouchBits|=I2C_ReadB(0x01)<<8;   
//		printf("irq is detect, %X\r\n",uwTouchBits);
    }
//	if(is_Motor_Moving())
//		return ;
    
    for(i=0; i<MAX_KEY_NUM; i++)
    {
        uwBit=(uwTouchBits>>i)&0x0001;
        if(uwBit)
        { 
			if(uwKeyStatus[i].flag==0)
			{
				uwKeyStatus[i].flag=1;
				uwKeyStatus[i].time = 0;
				uwKeyStatus[i].timebase = time;
				uwKeyStatus[i].ucKeyPrePress = 1;
			}

            if(uwKeyStatus[i].time<2*TOUCH_LONG_TIME)
            {
                uwKeyStatus[i].time = time - uwKeyStatus[i].timebase;
            }
            if(uwKeyStatus[i].time==TOUCH_LONG_TIME) 
            {    
				ucKey=ucKeyIndx[i] | LONG_KEY_MASK;
				evt.event = TOUCH_KEY_EVENT;
				evt.data.KeyValude = ucKey;
				PutEvent(evt);
				if(Get_fifo_size(&touch_key_fifo)==TOUCH_KEY_PSWD_MAX_LEN+1)
					fifo_clear(&touch_key_fifo);
				if(is_Motor_Moving()||(lock_operate.lock_state==LOCK_OPEN_NORMAL))
				{
					if(evt.data.KeyValude==('#'| LONG_KEY_MASK))
						ONE_WARM_BEEP();
				}
				else
					ONE_WARM_BEEP();	
				printf("long: %c, time=%d\r\n",ucKey&(~LONG_KEY_MASK), uwKeyStatus[i].time);
            }
        }
        else if(uwKeyStatus[i].ucKeyPrePress)
        {   
            if((uwKeyStatus[i].time>=TOUCH_SHORT_TIME)&&(uwKeyStatus[i].time<TOUCH_LONG_TIME))
            {
                ucKey=ucKeyIndx[i];
				evt.event = TOUCH_KEY_EVENT;
				evt.data.KeyValude = ucKey;
				PutEvent(evt);
				if((Get_fifo_size(&touch_key_fifo)==TOUCH_KEY_PSWD_MAX_LEN+1))
					fifo_clear(&touch_key_fifo);
				else
					fifo_in(&touch_key_fifo,ucKey);
				if(!(is_Motor_Moving()||(lock_operate.lock_state==LOCK_OPEN_NORMAL)))
					ONE_WARM_BEEP();
                printf("short: %c, time= %d\r\n",ucKey, uwKeyStatus[i].time);
            }
            uwKeyStatus[i].flag=0;
			uwKeyStatus[i].time=0;
			uwKeyStatus[i].timebase=0;
			uwKeyStatus[i].ucKeyPrePress=1;
        }
    }   
//    if(ucKeyPrePress)
//    {
//        ucKeyPrePress=0;
//    }
}



