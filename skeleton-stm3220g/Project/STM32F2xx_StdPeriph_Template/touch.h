#ifndef _TOUCH_H_
#define _TOUCH_H_
#include "stm322xg_eval.h"
#include "interface.h"
#include "LCD.h"
typedef struct {
  uint16_t lower, upper, left, right;
	Button_t button;
	void (*func)(void);
}TCallback;

void registerTCallback( 
	uint16_t left, 
	uint16_t right, 
	uint16_t lower, 
	uint16_t upper, 
	Button_t button,
	void (*func)(void)
);

TCallback *getCallbacks(void);

uint8_t getCallbackNum(void);
	
void leftToUp(void);

void rightToDown(void);	

void toggleOrange(void);

void setupTouch(uint8_t *graph_index);
	
#endif

