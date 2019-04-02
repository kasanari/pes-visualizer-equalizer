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


#include "stm322xg_eval.h"

typedef struct {
  u16 lower, upper, left, right;
  void *data;
  void (*callback)(u16 x, u16 y, u16 pressure, void *data);
} TSCallback;

extern TSCallback callbacks[64];
extern u8 callbackNum;

void registerTSCallback(u16 left, u16 right, u16 lower, u16 upper,
            void (*callback)(u16 x, u16 y, u16 pressure, void *data),
						void *data);		
						
