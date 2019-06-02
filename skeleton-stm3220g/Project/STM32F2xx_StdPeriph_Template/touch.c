#include "touch.h"
#include "queue.h"
#include "semphr.h"
TCallback callback[64];
uint8_t callbacksNum=0;
uint8_t *graph_index_touch;

TCallback *getCallbacks() {
	return callback;
}

uint8_t getCallbackNum() {
	return callbacksNum;
}

void registerTCallback(uint16_t left, uint16_t right, uint16_t lower, uint16_t upper, Button_t button, void (*func)(void)) {
  callback[callbacksNum].lower    = lower;
  callback[callbacksNum].upper    = upper;
  callback[callbacksNum].left     = left;
  callback[callbacksNum].right    = right;
	callback[callbacksNum].button   = button;
	callback[callbacksNum].func     = func;
  callbacksNum++;
}

void touchTask(void* params){
  portTickType lastWakeTime = xTaskGetTickCount();
  TS_STATE *ts_state; 
  u8 pressed = 0;
  u8 i;
  for (;;) {
    ts_state = IOE_TS_GetState();

	if (pressed) {
	  if (!ts_state->TouchDetected)
	    pressed = 0;
	} else if (ts_state->TouchDetected) {
	  for (i = 0; i < callbacksNum; ++i) {
		if (callback[i].left  <= ts_state->X &&
		    callback[i].right >= ts_state->X &&
		    callback[i].lower >= ts_state->Y &&
		    callback[i].upper <= ts_state->Y){
					callback[i].button.status = enable;
					callback[i].func();
				}	
	  }													
	  pressed = 1;
	}
	vTaskDelayUntil( &lastWakeTime, 100 / portTICK_RATE_MS);
}
}

void selectPrevVisuliser() {
	STM_EVAL_LEDToggle(LED4);
	if (*graph_index_touch == 0) {
		*graph_index_touch = NUM_OF_GRAPHS-1;
	} else {
		*graph_index_touch = (*graph_index_touch - 1) % NUM_OF_GRAPHS;
	}
}

void selectNextVisuliser(){
	STM_EVAL_LEDToggle(LED3);
	*graph_index_touch = (*graph_index_touch + 1 ) % NUM_OF_GRAPHS ; 
}

void toggleOrange(){
	STM_EVAL_LEDToggle(LED2);
}

void setupTouch(uint8_t *graph_i){ 
	graph_index_touch = graph_i; 
	xTaskCreate( touchTask, "touchscreen", 100, graph_i, 1, NULL);
}
