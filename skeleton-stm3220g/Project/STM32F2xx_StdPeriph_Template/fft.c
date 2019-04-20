//#include "arm_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#define FFT_LENGTH 256 				/* number of frequency bins/segments */

#define PI 3.141592653589793238462643383279502884197169399

//#define DEFAULT_VOLUME  70    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values      */                                        
																							
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


int FFT(double *real_in, double *imag_in, double *real_out, double *imag_out, uint16_t fft_length) {
	uint8_t levels = 0;
	uint8_t i, j, k;
	double w_m_r, w_m_i, w_i, w_r, t_r, t_i, u_i, u_r;
	uint8_t rev_index;
	uint8_t m;
	double *A_real = real_out;
	double *A_imag = imag_out;
	double w_r_temp;
	//uint16_t input[4] = {2, 1, 0, 1};
	
	for (i = fft_length; i > 1; i >>= 1) { // Take the two-logarithm of fft_length to determine number of steps
		levels++;
	}
	
	if ((1U << levels) != fft_length) { // Verify that fft_length is an exponential of 2
		return 0;
	}
	
	for (i = 0; i < fft_length; i++) {
		rev_index = reverse_bits(i, levels);
		A_real[rev_index] = real_in[i];
		A_imag[rev_index] = imag_in[i];
	}
	
	for (i = 0; i <= levels; i++) {
		m = 1 << i;
		
		// w_m = cos((2*PI)/m) - i*sin((2*PI)/m)
		w_m_r = cos((2*PI)/m);
		w_m_i = -sin((2*PI)/m); 

		for (k = 0; k < fft_length; k += m) {
			// w = 1
			w_r = 1;
			w_i = 0;
			for (j = 0; j < m/2; j++) {
				// Real part
				t_r = w_r*A_real[k + j + m/2] - w_i*A_imag[k + j + m/2];
				u_r = A_real[k + j];
				A_real[k + j] = u_r + t_r;
				A_real[k + j + m/2] = u_r - t_r;
				
				// Imaginary part
				t_i = w_r*A_imag[k + j + m/2] - w_i*A_real[k + j + m/2];
				u_i = A_imag[k + j];
				A_imag[k + j] = u_i + t_i;
				A_imag[k + j + m/2] = u_i - t_i;
				
				// Complex multiplication to get w = w*w_m
				w_r_temp = w_r; 										// Save a copy of the old w_r since it is used in both multiplications
				w_r = w_r_temp*w_m_r - w_i*w_m_i; 
        w_i = w_i*w_m_r 		 + w_r_temp*w_m_i;
			}
		}
	}
	
	return 1;
}

int complex_abs(double *real_in, double *imag_in, double *magnitude, uint16_t length) {
	for (int i = 0; i < length; i++) {
		magnitude[i] = sqrt(real_in[i]*real_in[i]+imag_in[i]*imag_in[i]);
	}
}

int inverse_FFT(double *real_in, double *imag_in, double *real_out, double *imag_out, uint16_t fft_length) {
	int status;
	int i;
	status = FFT(imag_in, real_in, imag_out, real_out, 4);
	if (status == 0) {
		return 0;
	} else {
		for (i = 0; i < fft_length; i++) {
			real_out[i] /= fft_length;
			imag_out[i] /= fft_length;
		}
		return 1;
	}
}

// Utility function to verify that two buffers contain the same values
bool buffer_compare(double *buf1, double *buf2, int n) {
	for (int i = 0; i < n; i++) {
		if (buf1[i] != buf2[i]) {
			return false;
		}
	}
	return true;
}

static bool check(uint8_t assertion, char *name) {
  if (!assertion) {
    printf("Test %s failed\n", name);
		return false;
  } else {
		printf("Test %s passed\n", name);
		return true;
	}
}

void print_buffer(double *buf, uint8_t n) {
	printf("[ ");
	for (int i = 0; i < n; i++) {
		printf("%f ", buf[i]);
	}
	printf("]\n");
}

void fft_test(double *real_in, double *imag_in, double *real_out_expected, double *imag_out_expected, bool* test_results, int n) {
	
	double *output_real = calloc(n, sizeof(double));
	double *output_imag = calloc(n, sizeof(double));
	
	double *inverse_real = calloc(n, sizeof(double));
	double *inverse_imag = calloc(n, sizeof(double));
	
	print_buffer(real_in, n);
	print_buffer(imag_in, n);

	// Check that the output is what we expect
	FFT(real_in, imag_in, output_real, output_imag, n);
	
	test_results[0] = check(buffer_compare(output_real, real_out_expected, n), "test_real");
	test_results[1] = check(buffer_compare(output_imag, imag_out_expected, n), "test_imag");
	print_buffer(output_real, n);
	print_buffer(output_imag, n);
	
	// Check that the inverse is equal to the original input
	inverse_FFT(output_real, output_imag, inverse_real, inverse_imag, n);

	test_results[2] = check(buffer_compare(real_in, inverse_real, n), "test_inverse_real");
	test_results[3] = check(buffer_compare(imag_in, inverse_imag, n), "test_inverse_imag");
	print_buffer(inverse_real, n);
	print_buffer(inverse_imag, n);

	// Cleanup
	free(output_real);
	free(output_imag);
	free(inverse_real);
	free(inverse_imag);
	
}

int main(int argc, char const *argv[])
{
	double input_real[4] = {0, 0, 0, 0};
	double input_imag[4] = {1, -1, 1, -1};
	double output_real_expected[4] = {0, 0, 0, 0};
	double output_imag_expected[4] = {1, -1, 1, -1};
	
	bool test_results[4] = {0, 0, 0, 0};
	
	fft_test(input_real, input_imag, output_real_expected, output_imag_expected, test_results, 4);
	return 0;
}
