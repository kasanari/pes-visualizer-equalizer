#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include "stm322xg_eval_ioe.h"
#include "FreeRTOS.h"
#include "task.h"

enum status { 
	disable = 0 , 
	enable = 1
};

enum titlebar {
	type = 0,
	mode = 1
 };

struct button { 
	int status ;
	char* info;
	uint16_t x,y,width,height;
}; 

typedef struct button Button_t; 

void setupInterface(void);

void writeTitle (char* ptr);

void LCD_write( uint16_t row, uint16_t x, char* ptr, uint8_t direction);

#endif

