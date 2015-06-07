#ifdef __DEBUG_H_
#define __DEBUG_H_
#include "stdio.h"


#ifdef _DEBUG_
	#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
	#define DEBUG(format, ...)
#endif


#endif


