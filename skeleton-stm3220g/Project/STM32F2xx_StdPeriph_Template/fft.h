#ifndef _FFT_H
#define _FFT_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "global.h"
#include <stdbool.h>

typedef struct FFT_signals {
	FLOAT_TYPE *real_input;
	FLOAT_TYPE *imag_input;
	FLOAT_TYPE *real_output;
	FLOAT_TYPE *imag_output;
	FLOAT_TYPE *magnitude;
	xSemaphoreHandle fft_done_lock;
	xSemaphoreHandle graph_done_lock;
} FFT_signals_t;


bool FFT_Init(int n);

bool FFT(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *real_out, FLOAT_TYPE *imag_out, uint16_t fft_length);

void complex_abs(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *magnitude, uint16_t length);

bool inverse_FFT(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *real_out, FLOAT_TYPE *imag_out, uint16_t fft_length);

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals);

#endif
