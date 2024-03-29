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
																									
static uint8_t bit_rev_table[FFT_LENGTH];
static FLOAT_TYPE sine_table[FFT_LENGTH/2];
static FLOAT_TYPE cosine_table[FFT_LENGTH/2];
xSemaphoreHandle buffer_switch_lock;

xSemaphoreHandle buffer1_lock;
xSemaphoreHandle buffer2_lock;

xSemaphoreHandle fft_input_lock;

uint16_t audio_buffer_1[FFT_LENGTH];
uint16_t audio_buffer_2[FFT_LENGTH];
uint16_t offset[FFT_LENGTH];

uint16_t fft_input[FFT_LENGTH];

uint8_t	 buffer_select = 0;

bool init = true;

void copy_values(uint16_t *dest, uint16_t *src, size_t n) {
	for (int i = 0; i < n; i++) {
		dest[i] = src[i];
	}
}

uint32_t STM32_AudioRec_Update(uint8_t** pbuf, uint32_t* pSize) {
	static BaseType_t xHigherPriorityTaskWoken;
	*pSize = FFT_LENGTH;
	
	if (init) {
		copy_values(offset, audio_buffer_1, FFT_LENGTH);
		init = false;
	}
	
	if (buffer_select == 0)  { // buf1 has been filled with values
		if ( xSemaphoreTakeFromISR(buffer2_lock, &xHigherPriorityTaskWoken) == pdTRUE ) { // Check if buf2 is available for writing
				 *pbuf = (uint8_t*)audio_buffer_2; // If it is, switch to writing to buf2
					buffer_select = 1; // indicate buf1 as available for reading
					xSemaphoreGiveFromISR(buffer1_lock, &xHigherPriorityTaskWoken); // Set buf1 as available
       } else {
				 *pbuf = (uint8_t*)audio_buffer_1; // Otherwise, keep writing to buf1
			 } 
	} else { // buf2 has been filled with values
			if ( xSemaphoreTakeFromISR( buffer1_lock, &xHigherPriorityTaskWoken) == pdTRUE ) { // Check if buf1 is available
				 *pbuf = (uint8_t*)audio_buffer_1; // If it is, switch to writing to buf1
					buffer_select = 0; // indicate buf2 as available for reading
					xSemaphoreGiveFromISR(buffer2_lock, &xHigherPriorityTaskWoken); // Set buf2 as available
      } else {
				 *pbuf = (uint8_t*)audio_buffer_2; // Otherwise, keep writing to buf2
			}
	}
	
	
	xSemaphoreGiveFromISR(buffer_switch_lock, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken);
	
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

void complex_mul(FLOAT_TYPE a, FLOAT_TYPE b, FLOAT_TYPE c, FLOAT_TYPE d, FLOAT_TYPE *result_r, FLOAT_TYPE *result_i) {
	*result_r = a*c - b*d;
	*result_i = a*d + b*c;
}

bool FFT_Init(int n) {
	int levels = 0;
	int i, m;
	for (i = n; i > 1; i >>= 1) {
		levels++;
	}

	if ((1U << levels) != n) { // Verify that fft_length is an exponential of 2
		return false;
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

bool FFT_Int(uint16_t *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *real_out, FLOAT_TYPE *imag_out, uint16_t fft_length) {
	uint8_t levels = 0;
	uint8_t i, j, k;
	FLOAT_TYPE w_m_r, w_m_i, w_i, w_r, t_r, t_i, u_i, u_r;
	uint8_t rev_index;
	uint8_t m;
	FLOAT_TYPE *A_real = real_out;
	FLOAT_TYPE *A_imag = imag_out;
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

bool FFT(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *real_out, FLOAT_TYPE *imag_out, uint16_t fft_length) {
	uint8_t levels = 0;
	uint8_t i, j, k;
	FLOAT_TYPE w_m_r, w_m_i, w_i, w_r, t_r, t_i, u_i, u_r;
	uint8_t rev_index;
	uint8_t m;
	FLOAT_TYPE *A_real = real_out;
	FLOAT_TYPE *A_imag = imag_out;
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

void complex_abs(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *magnitude, uint16_t length) {
	for (int i = 0; i < length; i++) {
		magnitude[i] = sqrtf(real_in[i]*real_in[i]+imag_in[i]*imag_in[i]);
	}
}


bool inverse_FFT(FLOAT_TYPE *real_in, FLOAT_TYPE *imag_in, FLOAT_TYPE *real_out, FLOAT_TYPE *imag_out, uint16_t fft_length) {
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

void copy_values_offset(uint16_t *dest, uint16_t *src, uint16_t *offset, size_t n) {
	int temp;
	for (int i = 0; i<n; i++) {
		temp = src[i] - offset[i];
		dest[i] = (temp < 0) ? 0 : temp;
	}
}

static void FFTTask(void *params) {
		volatile FFT_signals_t *signals = (FFT_signals_t*)params;
		bool status;
		if (FFT_Init(FFT_LENGTH)) {
	for (;;) {
		
				xSemaphoreTake(buffer_switch_lock, portMAX_DELAY);
		
				if (buffer_select == 1) {
					xSemaphoreTake(buffer1_lock, portMAX_DELAY); // Set buf1 as in use
					copy_values_offset(fft_input, audio_buffer_1, offset, FFT_LENGTH);
					xSemaphoreGive(buffer1_lock); // Give access to buf1
				} else {
					xSemaphoreTake(buffer2_lock, portMAX_DELAY); // Set buf2 as in use
					copy_values_offset(fft_input, audio_buffer_2, offset, FFT_LENGTH);
					xSemaphoreGive(buffer2_lock); // Give access to buf2
				}
					status = FFT_Int(fft_input, signals->imag_input, signals->real_output, signals->imag_output, FFT_LENGTH);

				if (status == false) {
					printf("FFT failed");
				} else {
					xSemaphoreTake(signals->graph_done_lock, portMAX_DELAY);
					complex_abs(signals->real_output, signals->imag_output, signals->magnitude, FFT_LENGTH/2);
					xSemaphoreGive(signals->fft_done_lock);
				}
	}
		} else {
			printf("FFT initialization failed");
		}

	vTaskDelay(portMAX_DELAY);
}

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals) {
	buffer_switch_lock = xSemaphoreCreateBinary();
	buffer1_lock = xSemaphoreCreateBinary();
	buffer2_lock = xSemaphoreCreateBinary();
	xSemaphoreGive(buffer1_lock);
	xSemaphoreGive(buffer2_lock);
	STM32_AudioRec_Init(SAMPLE_FREQ, DEFAULT_IN_BIT_RESOLUTION, DEFAULT_IN_CHANNEL_NBR);
	if (buffer_select == 0) {
		STM32_AudioRec_Start((uint8_t*)audio_buffer_1, FFT_LENGTH);
	} else {
		STM32_AudioRec_Start((uint8_t*)audio_buffer_2, FFT_LENGTH);
	}
	
	BaseType_t xReturned;
	xReturned = xTaskCreate(FFTTask, "fft", 1000, signals, uxPriority, NULL);
	if( xReturned == pdPASS ) {
		printf("FFT task created!");
	} else {
		printf("FFT task could not be created!");
	}
}
