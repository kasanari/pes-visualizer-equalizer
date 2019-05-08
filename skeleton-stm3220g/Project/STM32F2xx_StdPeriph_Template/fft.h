#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#ifndef _FFT_H
#define _FFT_H

typedef struct FFT_signals {
	uint16_t size;
	float *real_input;
	float *imag_input;
	float *real_output;
	float *imag_output;
	float *magnitude;
	xSemaphoreHandle fft_done_lock;
	xSemaphoreHandle graph_done_lock;
} FFT_signals_t;

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals);

#endif
