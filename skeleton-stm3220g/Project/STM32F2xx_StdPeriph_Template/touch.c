#include "touch.h"
#include "queue.h"
#include "semphr.h"
TCallback callback[64];
uint8_t callbacksNum=0;
char title[3][13] = {"SimpleWhite ", "SimpleRainbo", "BlockRainbo "};
uint8_t title_index = 0;

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

void leftToUp() {
	STM_EVAL_LEDToggle(LED4);
	if (title_index == 0) {
		title_index = 2;
	} else {
		title_index = (title_index - 1) % 3;
	}
	writeTitle(title[title_index]);
}

void rightToDown(){
	STM_EVAL_LEDToggle(LED2);
	title_index = (title_index + 1 ) % 3 ; 
	writeTitle(title[title_index]);
}

void toggleOrange(){
	STM_EVAL_LEDToggle(LED2);
}

void setupTouch(void){
	xTaskCreate( touchTask, "touchscreen", 100, NULL, 1, NULL);
}
