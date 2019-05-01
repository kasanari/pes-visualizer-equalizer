#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include "stm322xg_eval_ioe.h"
#include "FreeRTOS.h"
#include "task.h"

enum status { 
	disable = 0 , 
	enable = 1
};

struct button { 
	int status ; 
}; 

typedef struct button Button_t; 

void setupInterface(void);

#endif

