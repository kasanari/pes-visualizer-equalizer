#ifndef _FFT_H
#define _FFT_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdbool.h>

typedef struct FFT_signals {
	float *real_input;
	float *imag_input;
	float *real_output;
	float *imag_output;
	float *magnitude;
	xSemaphoreHandle fft_done_lock;
	xSemaphoreHandle graph_done_lock;
} FFT_signals_t;


bool FFT_Init(int n);

bool FFT(float *real_in, float *imag_in, float *real_out, float *imag_out, uint16_t fft_length);

void complex_abs(float *real_in, float *imag_in, float *magnitude, uint16_t length);

bool inverse_FFT(float *real_in, float *imag_in, float *real_out, float *imag_out, uint16_t fft_length);

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals);

#endif
