/*----------------------------------------------------------------------------
 * Name:    TS.h
 * Purpose: TouchScreen API for registering callback functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * The API is provided as part of the Programming Embedded Systems course 
 * at Uppsala University.
 * This software is supplied "AS IS" without warranties of any kind.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/
#include "TS.h"

TSCallback callbacks[64];
u8 callbackNum=0;


/**
 * Register a callback that will be invoked when a touch screen
 * event occurs within a given rectangle
 *
 * NB: the callback function should have a short execution time,
 * since long-running callbacks will prevent further events from
 * being generated
 */

void registerTSCallback(u16 left, u16 right, u16 lower, u16 upper,
                        void (*callback)(u16 x, u16 y, u16 pressure, void *data),
						void *data) {
  callbacks[callbackNum].lower    = lower;
  callbacks[callbackNum].upper    = upper;
  callbacks[callbackNum].left     = left;
  callbacks[callbackNum].right    = right;
  callbacks[callbackNum].callback = callback;
  callbacks[callbackNum].data     = data;
  callbackNum++;
}

/* An example of a touchScreenTask that executes callback functions */
/*
			void touchScreenTask(void *params) {
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
					for (i = 0; i < callbackNum; ++i) {
					if (callbacks[i].left  <= ts_state->X &&
							callbacks[i].right >= ts_state->X &&
							callbacks[i].lower >= ts_state->Y &&
							callbacks[i].upper <= ts_state->Y)
						callbacks[i].callback(ts_state->X, ts_state->Y, ts_state->Z,
																	callbacks[i].data);
					}													
					pressed = 1;
				}

					if (ts_state->TouchDetected) {
					printf("%d,%d,%d ", ts_state->X, ts_state->Y, ts_state->Z);
				}

				vTaskDelayUntil(&lastWakeTime, 100 / portTICK_RATE_MS);
				}
			}
*/
