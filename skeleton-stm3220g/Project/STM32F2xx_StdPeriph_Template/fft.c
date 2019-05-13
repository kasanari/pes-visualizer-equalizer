//#include "arm_math.h"
#include "stm32f2xx_adc.h"
#include "stm32_audio.h"
#include "stm322xg_audio_recorder.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "global.h"

#define PI 3.141592653589793238462643383279502884197169399

//#define DEFAULT_VOLUME  70    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values      */                                        
																									
static float inverse_real[FFT_LENGTH];
static float inverse_imag[FFT_LENGTH];
static uint8_t bit_rev_table[FFT_LENGTH];
xSemaphoreHandle buffer_switch_sem;

uint16_t* audio_buffer_1;
uint16_t* audio_buffer_2;

uint8_t	 buffer_select = 1;

uint32_t STM32_AudioRec_Update(uint8_t** pbuf, uint32_t* pSize) {
	//portBASE_TYPE higherPrio;
	*pSize = FFT_LENGTH;
  *pbuf = (buffer_select) ? (uint8_t*)audio_buffer_1 : (uint8_t*)audio_buffer_2;
	/* Toggle the buffer select */
  buffer_select = (buffer_select == 0) ? 1 : 0;
	
	//xSemaphoreGiveFromISR(buffer_switch_sem, &higherPrio);
	//vPortYieldFromISR();
	
	return 0;
}

/*Reverse a number of bits*/
static uint16_t reverse_bits(uint16_t bits, uint16_t n_bits) {
	uint16_t bits_reversed = 0; 		// To store the reversed bits
	uint16_t i;
	for (i = 0; i < n_bits; i++) {
		bits_reversed <<= 1; 					// Shift the result to the left
		bits_reversed |= (bits & 1U); // Add the current last bit of bits.
		bits >>= 1; 									// Shift the original bits to the right
	}
	return bits_reversed;
}

// (a + ib)(c + id) = (ac - bd) + i(ad + bc)

void complex_mul(float a, float b, float c, float d, float *result_r, float *result_i) {
	*result_r = a*c - b*d;
	*result_i = a*d + b*c;
}

float *sine_table = 0;
float *cosine_table = 0;

bool FFT_Init(int n) {
	int levels = 0;
	int i, m;
	for (i = n; i > 1; i >>= 1) {
		levels++;
	}

	if ((1U << levels) != n) { // Verify that fft_length is an exponential of 2
		return false;
	}

	sine_table = calloc(levels, sizeof(float));
	cosine_table = calloc(levels, sizeof(float));

	if ((sine_table == NULL) || (cosine_table == NULL)) {
		return false; // Could not allocate enough memory
	}
	
	for (i = 0; i < levels; i++) {
		m = 1 << (i+1);
		cosine_table[i] = cos((2*PI)/m);
		sine_table[i] =  -sin((2*PI)/m);
	}

	for (i = 0; i<n; i++) {
		bit_rev_table[i] = reverse_bits(i, levels);
	}

	return true;

}

bool FFT(float *real_in, float *imag_in, float *real_out, float *imag_out, uint16_t fft_length) {
	uint8_t levels = 0;
	uint8_t i, j, k;
	float w_m_r, w_m_i, w_i, w_r, t_r, t_i, u_i, u_r;
	uint8_t rev_index;
	uint8_t m;
	float *A_real = real_out;
	float *A_imag = imag_out;
	//uint16_t input[4] = {2, 1, 0, 1};
	
	for (i = fft_length; i > 1; i >>= 1) { // Take the two-logarithm of fft_length to determine number of steps
		levels++;
	}
	
	if ((1U << levels) != fft_length) { // Verify that fft_length is an exponential of 2
		return false;
	}
	
	for (i = 0; i < fft_length; i++) {
		rev_index = bit_rev_table[i];
		A_real[rev_index] = real_in[i];
		A_imag[rev_index] = imag_in[i];
	}
	
	for (i = 0; i < levels; i++) {
		m = 1 << (i+1);
		
		// w_m = cos((2*PI)/m) - i*sin((2*PI)/m)
		w_m_r = cosine_table[i];
		w_m_i = sine_table[i]; 

		for (k = 0; k < fft_length; k += m) {
			// w = 1
			w_r = 1;
			w_i = 0;
			for (j = 0; j < m/2; j++) {

				complex_mul(w_r, w_i, A_real[k + j + m/2], A_imag[k + j + m/2], &t_r, &t_i);

				// Real part
				u_r = A_real[k + j];
				A_real[k + j] = u_r + t_r;
				A_real[k + j + m/2] = u_r - t_r;
				
				// Imaginary part
				u_i = A_imag[k + j];
				A_imag[k + j] = u_i + t_i;
				A_imag[k + j + m/2] = u_i - t_i;
				
				// Complex multiplication to get w = w*w_m
				
				complex_mul(w_r, w_i, w_m_r, w_m_i, &w_r, &w_i);

			}
		}
	}
	
	return true;
}

void complex_abs(float *real_in, float *imag_in, float *magnitude, uint16_t length) {
	for (int i = 0; i < length; i++) {
		magnitude[i] = sqrt(real_in[i]*real_in[i]+imag_in[i]*imag_in[i]);
	}
}


bool inverse_FFT(float *real_in, float *imag_in, float *real_out, float *imag_out, uint16_t fft_length) {
	bool status;
	int i;
	status = FFT(imag_in, real_in, imag_out, real_out, fft_length);
	if (status == false) {
		return false;
	} else {
		for (i = 0; i < fft_length; i++) {
			real_out[i] /= fft_length;
			imag_out[i] /= fft_length;
		}
		return true;
	}
}

static void FFTTask(void *params) {
		volatile FFT_signals_t *signals = (FFT_signals_t*)params;
		bool status;

		if (FFT_Init(signals->size)) {
	for (;;) {
				status = FFT(signals->real_input, signals->imag_input, signals->real_output, signals->imag_input, signals->size);
				if (status == false) {
					printf("FFT failed");
				} else {
					xSemaphoreTake(signals->graph_done_lock, portMAX_DELAY);
			complex_abs(signals->real_output, signals->imag_output, signals->magnitude, signals->size);
					xSemaphoreGive(signals->fft_done_lock);
				}
			vTaskDelay(30 / portTICK_RATE_MS);
	}
		} else {
			printf("FFT initialization failed");
		}

	vTaskDelay(portMAX_DELAY);
}

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals) {
	BaseType_t xReturned;
	xReturned = xTaskCreate(testFFTTask, "fft", 1000, signals, uxPriority, NULL);
	if( xReturned == pdPASS ) {
		printf("FFT task created!");
	} else {
		printf("FFT task could not be created!");
	}
}
