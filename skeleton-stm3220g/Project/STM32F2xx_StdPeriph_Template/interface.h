#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include "stm322xg_eval_ioe.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "graph.h"

typedef struct graph_type {
	drawGraphFunction graph_func;
	char* name;
} graph_type_t;

typedef struct context {
	xSemaphoreHandle lcd_lock;
	graph_type_t *graphs;
	uint8_t *graph_index;
} context_t;

enum status { 
	disable = 0 , 
	enable = 1
};

struct button { 
	int status ;
	char* name;
	uint16_t x,y,width,height;
}; 


typedef struct button Button_t; 

void setupInterface(context_t *ctx);

void writeTitle (char* ptr);

void LCD_write( uint16_t row, uint16_t x, char* ptr, uint8_t direction);

#endif

