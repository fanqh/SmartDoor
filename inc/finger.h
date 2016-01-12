#ifndef _FINGER_H_
#define _FINGER_H_


#define ACK_SUCCESS      0x00 
#define ACK_FAIL     	 0x01 
#define ACK_FULL       	 0x04  
#define ACK_NOUSER     	 0x05  
#define ACK_USER_EXIST   0x07  
#define ACK_TIMEOUT  	 0x08 


#define MATCH_CMD 	0X0D
#define REGIST1_CMD 0X01
#define REGIST2_CMD 0X02
#define REGIST3_CMD 0X03

void finger_init(void);

#endif
