//#include "link_list.h"
#include "iic.h"
#include "mpr121_key.h"

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

uint8_t I2C_ReadB(uint8_t     iAddress)
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

#if 0

#define SCAN_FREQ       10

struct node_struct_t touch_key_scan;

uint8_t touch_key_buf[16];
fifo_struct touch_key_fifo;

void clr_touch_key_buf(void)
{
    fifo_clear(&touch_key_fifo);
}

int16_t get_touch_key(uint8_t *puckeybuf, uint32_t time_out)
{
    uint32  i,tv=time_out;
    int16_t   result;

    while(1)
    {
        result=fifo_out(&touch_key_fifo,puckeybuf);
        if(result==0)
            return EF_SUCCESS;
        if(time_out)
        {
            tv--;
            if(tv==0)
                return EF_TIME_OUT;
        }
        ms_delay(1);
    }
    return EF_FAIL;
}

void mpr121_scan(void *priv)
{   
    uint32  uiStatus;
    
    uiStatus=Sys_GetStableIoStatus(GPIOB,MPR121_IRQ_PIN);    
    if(uiStatus)
        fifo_in(&touch_key_fifo,I2C_ReadB(0x00));
}
#endif
void mpr121_init_config(void)
{
    I2C_Write(MHD_R, 0x01);
    I2C_Write(NHD_R, 0x01);
    I2C_Write(NCL_R, 0x00);
    I2C_Write(FDL_R, 0x00);

    // Section B
    // This group controls filtering when data is < baseline.
    I2C_Write(MHD_F, 0x01);
    I2C_Write(NHD_F, 0x01);
    I2C_Write(NCL_F, 0xFF);
    I2C_Write(FDL_F, 0x02);

    // Section C
    // This group sets touch and release thresholds for each electrode
    I2C_Write(ELE0_T, TOU_THRESH);
    I2C_Write(ELE0_R, REL_THRESH);
    I2C_Write(ELE1_T, TOU_THRESH);
    I2C_Write(ELE1_R, REL_THRESH);
    I2C_Write(ELE2_T, TOU_THRESH);
    I2C_Write(ELE2_R, REL_THRESH);
    I2C_Write(ELE3_T, TOU_THRESH);
    I2C_Write(ELE3_R, REL_THRESH);
    I2C_Write(ELE4_T, TOU_THRESH);
    I2C_Write(ELE4_R, REL_THRESH);
    I2C_Write(ELE5_T, TOU_THRESH);
    I2C_Write(ELE5_R, REL_THRESH);
    I2C_Write(ELE6_T, TOU_THRESH);
    I2C_Write(ELE6_R, REL_THRESH);
    I2C_Write(ELE7_T, TOU_THRESH);
    I2C_Write(ELE7_R, REL_THRESH);
    I2C_Write(ELE8_T, TOU_THRESH);
    I2C_Write(ELE8_R, REL_THRESH);
    I2C_Write(ELE9_T, TOU_THRESH);
    I2C_Write(ELE9_R, REL_THRESH);
    I2C_Write(ELE10_T, TOU_THRESH);
    I2C_Write(ELE10_R, REL_THRESH);
    I2C_Write(ELE11_T, TOU_THRESH);
    I2C_Write(ELE11_R, REL_THRESH);

    // Section D
    // Set the Filter Configuration
    // Set ESI2
    I2C_Write(FIL_CFG, 0x04);

    // Section E
    // Electrode Configuration
    // Enable 6 Electrodes and set to run mode
    // Set ELE_CFG to 0x00 to return to standby mode
    // I2C_Write(ELE_CFG, 0x0C);	// Enables all 12 Electrodes
    I2C_Write(ELE_CFG, 0x06);		// Enable first 6 electrodes

    // Section F
    // Enable Auto Config and auto Reconfig
    I2C_Write(ATO_CFG0, 0x0B);
    I2C_Write(ATO_CFGU, 0xC9);	// USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   I2C_Write(ATO_CFGL, 0x82);	// LSL = 0.65*USL = 0x82 @3.3V
    I2C_Write(ATO_CFGL, 0x82);
    I2C_Write(ATO_CFGT, 0xB5);	// Target = 0.9*USL = 0xB5 @3.3V

//    fifo_create(&touch_key_fifo,touch_key_fifo,sizeof(touch_key_fifo));
//    lklt_insert(&touch_key_scan,mpr121_scan,NULL,1000/SCAN_FREQ);

}


