#ifndef LPCD_API_H
#define LPCD_API_H

#include "lpcd_reg.h"
#include "lpcd_config.h"
#include "lpcd_config_user.h"
#include "lpcd_regctrl.h"
#include "RF1356.h"


#ifndef TRUE
#define	TRUE	1
#endif

#ifndef FALSE
	#define FALSE	0
#endif
extern unsigned char LpcdParamInit(void);
extern unsigned char LpcdRegisterInit(void);
extern unsigned char LpcdInitCalibra(unsigned char *CalibraFlag);


extern unsigned char ADCResultCenter;		//LPCDÂö¿íÖÐÐÄµã

uint8_t LPCD_IRQ_int(void);
extern void LpcdCardIn_IRQHandler(void);
extern void LpcdAutoWakeUp_IRQHandler(void);

void close_lpcd(void);

//extern unsigned char LpcdAuxSelect(unsigned char OpenClose);
//extern unsigned char LpcdAutoTest(void);

#endif


