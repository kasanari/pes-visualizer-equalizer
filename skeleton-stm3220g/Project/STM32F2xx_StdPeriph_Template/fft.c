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

#define FFT_LENGTH 256 				/* number of frequency bins/segments */

#define PI 3.141592653589793238462643383279502884197169399

//#define DEFAULT_VOLUME  70    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values      */                                        
																									
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
int *bit_rev_table = 0;

void FFT_Init(int n) {
	int levels = 0;
	int i, m;
	for (i = n; i > 1; i >>= 1) {
		levels++;
	}

	if (cosine_table != 0) {
		free(cosine_table);
	}
	if (sine_table != 0) {
		free(sine_table);
	}

	sine_table = calloc(levels+1, sizeof(float));
	cosine_table = calloc(levels+1, sizeof(float));
	bit_rev_table = calloc(n, sizeof(int));

	for (i = 0; i <= levels; i++) {
		m = 1 << i;
		cosine_table[i] = cos((2*PI)/m);
		sine_table[i] =  -sin((2*PI)/m);
	}
	

	for (i = 0; i <= levels; i++) {
		m = 1 << i;
		cosine_table[i] = cos((2*PI)/m);
		sine_table[i] =  -sin((2*PI)/m);
	}

	for (i = 0; i<n; i++) {
		bit_rev_table[i] = reverse_bits(i, levels);
	}


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
	
	for (i = 0; i <= levels; i++) {
		m = 1 << i;
		
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

bool float_compare(float a, float b, float epsilon) {
	return fabs(a - b) < epsilon;
}

// Utility function to verify that two buffers contain the same values
bool buffer_compare(float *buf1, float *buf2, int n) {
	for (int i = 0; i < n; i++) {
		bool equal = float_compare(buf1[i], buf2[i], 0.00005);
		if (!equal) {
			return false;
		}
	}
	return true;
}

bool check_test_result(bool *test_result) {
	for (int i = 0; i < 4; i++) {
		if (test_result[i] != true) {
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

void print_buffer(float *buf, uint8_t n) {
	printf("[ ");
	for (int i = 0; i < n; i++) {
		printf("%f ", buf[i]);
	}
	printf("]\n");
}
	
void fft_test(float *real_in, float *imag_in, float *real_out_expected, float *imag_out_expected, bool* test_results, int n) {
	
	float *output_real = calloc(n, sizeof(float));
	float *output_imag = calloc(n, sizeof(float));
	
	float *inverse_real = calloc(n, sizeof(float));
	float *inverse_imag = calloc(n, sizeof(float));
	

	// Check that the output is what we expect
	FFT(real_in, imag_in, output_real, output_imag, n);
	
	test_results[0] = check(buffer_compare(output_real, real_out_expected, n), "test_real");
	test_results[1] = check(buffer_compare(output_imag, imag_out_expected, n), "test_imag");


	// Check that the inverse is equal to the original input
	inverse_FFT(output_real, output_imag, inverse_real, inverse_imag, n);

	test_results[2] = check(buffer_compare(real_in, inverse_real, n), "test_inverse_real");
	test_results[3] = check(buffer_compare(imag_in, inverse_imag, n), "test_inverse_imag");
	
	// Cleanup
	free(output_real);
	free(output_imag);
	free(inverse_real);
	free(inverse_imag);
	
}

// Impulse
bool test_1() {
	FFT_Init(4);
	float input_real[4] = {1, 0, 0, 0};
	float input_imag[4] = {0, 0, 0, 0};
	float expected_output_real[4] = {1, 1, 1, 1};
	float expected_output_imag[4] = {0, 0, 0, 0};
	
	bool test_results[4] = {0, 0, 0, 0};
	fft_test(input_real, input_imag, expected_output_real, expected_output_imag, test_results, 4);
	return check_test_result(test_results);
}
	
// sin(x)
bool test_2() {
	FFT_Init(64);
	float input_real[64] = {0.000000, 0.099568, 0.198146, 0.294755, 0.388435, 0.478254, 0.563320, 0.642788, 0.715867, 0.781831, 0.840026, 0.889872, 0.930874, 0.962624, 0.984808, 0.997204, 0.999689, 0.992239, 0.974928, 0.947927, 0.911506, 0.866025, 0.811938, 0.749781, 0.680173, 0.603804, 0.521435, 0.433884, 0.342020, 0.246757, 0.149042, 0.049846, -0.049846, -0.149042, -0.246757, -0.342020, -0.433884, -0.521435, -0.603804, -0.680173, -0.749781, -0.811938, -0.866025, -0.911506, -0.947927, -0.974928, -0.992239, -0.999689, -0.997204, -0.984808, -0.962624, -0.930874, -0.889872, -0.840026, -0.781831, -0.715867, -0.642788, -0.563320, -0.478254, -0.388435, -0.294755, -0.198146, -0.099568, -0.000000};
	float input_imag[64] = {0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000};
	float expected_output_real[64] = {-0.000000, 1.557136, -0.067150, -0.056279, -0.053261, -0.051971, -0.051296, -0.050898, -0.050643, -0.050470, -0.050347, -0.050256, -0.050188, -0.050135, -0.050093, -0.050060, -0.050033, -0.050010, -0.049992, -0.049976, -0.049963, -0.049953, -0.049943, -0.049936, -0.049929, -0.049924, -0.049919, -0.049916, -0.049913, -0.049911, -0.049909, -0.049908, -0.049908, -0.049908, -0.049909, -0.049911, -0.049913, -0.049916, -0.049919, -0.049924, -0.049929, -0.049936, -0.049943, -0.049953, -0.049963, -0.049976, -0.049992, -0.050010, -0.050033, -0.050060, -0.050093, -0.050135, -0.050188, -0.050256, -0.050347, -0.050470, -0.050643, -0.050898, -0.051296, -0.051971, -0.053261, -0.056279, -0.067150, 1.557136};
	float expected_output_imag[64] = {0.000000, -31.696241, 0.681785, 0.379400, 0.267760, 0.207480, 0.169102, 0.142251, 0.122263, 0.106710, 0.094192, 0.083848, 0.075112, 0.067599, 0.061039, 0.055232, 0.050033, 0.045327, 0.041027, 0.037065, 0.033385, 0.029940, 0.026695, 0.023618, 0.020681, 0.017863, 0.015143, 0.012503, 0.009928, 0.007404, 0.004916, 0.002452, 0.000000, -0.002452, -0.004916, -0.007404, -0.009928, -0.012503, -0.015143, -0.017863, -0.020681, -0.023618, -0.026695, -0.029940, -0.033385, -0.037065, -0.041027, -0.045327, -0.050033, -0.055232, -0.061039, -0.067599, -0.075112, -0.083848, -0.094192, -0.106710, -0.122263, -0.142251, -0.169102, -0.207480, -0.267760, -0.379400, -0.681785, 31.696241};
		
	bool test_results[4] = {0, 0, 0, 0};
	fft_test(input_real, input_imag, expected_output_real, expected_output_imag, test_results, 64);
	return check_test_result(test_results);
}
		
bool test_3() {
	FFT_Init(64);
	float input_real[64] = {1.000000, 0.810002, 0.312205, -0.304228, -0.805056, -0.999965, -0.814890, -0.320160, 0.296230, 0.800054, 0.999859, 0.819721, 0.328092, -0.288211, -0.794995, -0.999684, -0.824495, -0.336001, 0.280172, 0.789881, 0.999438, 0.829211, 0.343887, -0.272113, -0.784711, -0.999121, -0.833868, -0.351748, 0.264035, 0.779486, 0.998735, 0.838467, 0.359585, -0.255939, -0.774206, -0.998278, -0.843007, -0.367396, 0.247824, 0.768872, 0.997751, 0.847488, 0.375181, -0.239692, -0.763484, -0.997154, -0.851909, -0.382941, 0.231544, 0.758042, 0.996487, 0.856270, 0.390673, -0.223379, -0.752547, -0.995749, -0.860571, -0.398377, 0.215198, 0.746999, 0.994942, 0.864811, 0.406054, -0.207002};
	float input_imag[64] = {0.000000, 0.586428, 0.950015, 0.952599, 0.593199, 0.008385, -0.579615, -0.947364, -0.955117, -0.599928, -0.016769, 0.572762, 0.944646, 0.957567, 0.606615, 0.025152, -0.565869, -0.941862, -0.959950, -0.613260, -0.033533, 0.558936, 0.939011, 0.962265, 0.619861, 0.041912, -0.551963, -0.936095, -0.964513, -0.626419, -0.050288, 0.544952, 0.933112, 0.966693, 0.632933, 0.058661, -0.537903, -0.930065, -0.968805, -0.639402, -0.067029, 0.530815, 0.926951, 0.970849, 0.645827, 0.075393, -0.523691, -0.923773, -0.972825, -0.652206, -0.083751, 0.516529, 0.920530, 0.974732, 0.658539, 0.092103, -0.509331, -0.917222, -0.976570, -0.664826, -0.100449, 0.502098, 0.913849, 0.978341};
	float expected_output_real[64] = {1.906317, 2.110843, 2.406120, 2.872661, 3.726112, 5.805854, 18.719448, -10.202924, -3.347429, -1.726765, -0.999162, -0.584329, -0.315204, -0.125710, 0.015532, 0.125341, 0.213541, 0.286258, 0.347511, 0.400048, 0.445811, 0.486213, 0.522309, 0.554901, 0.584613, 0.611936, 0.637263, 0.660917, 0.683161, 0.704216, 0.724269, 0.743480, 0.761989, 0.779917, 0.797376, 0.814463, 0.831271, 0.847887, 0.864391, 0.880866, 0.897389, 0.914042, 0.930908, 0.948074, 0.965633, 0.983687, 1.002349, 1.021744, 1.042016, 1.063332, 1.085884, 1.109904, 1.135668, 1.163512, 1.193849, 1.227201, 1.264225, 1.305777, 1.352983, 1.407362, 1.471016, 1.546938, 1.639546, 1.755649};
	float expected_output_imag[64] = {2.351868, 2.882607, 3.648845, 4.859505, 7.074188, 12.471064, 45.981498, -29.071291, -11.281463, -7.075883, -5.187772, -4.111291, -3.412919, -2.921189, -2.554667, -2.269716, -2.040839, -1.852141, -1.693191, -1.556860, -1.438106, -1.333263, -1.239595, -1.155019, -1.077918, -1.007017, -0.941292, -0.879911, -0.822189, -0.767552, -0.715515, -0.665662, -0.617633, -0.571109, -0.525805, -0.481464, -0.437847, -0.394730, -0.351901, -0.309151, -0.266273, -0.223059, -0.179292, -0.134748, -0.089182, -0.042332, 0.006094, 0.056424, 0.109030, 0.164344, 0.222868, 0.285199, 0.352055, 0.424308, 0.503034, 0.589579, 0.685657, 0.793483, 0.915982, 1.057095, 1.222274, 1.419289, 1.659605, 1.960888};
		
	bool test_results[4] = {0, 0, 0, 0};
	fft_test(input_real, input_imag, expected_output_real, expected_output_imag, test_results, 64);
	return check_test_result(test_results);
}

void testFFTTask(void *params) {
	volatile bool test1_result;
	for (;;) {
			test1_result = test_1();
			vTaskDelay(30 / portTICK_RATE_MS);
	}

	vTaskDelay(portMAX_DELAY);
}

void FFTTask(void *params) {
	FFT_signals_t *signals = (FFT_signals_t*)params;
	FFT_Init(signals->size);
	for (;;) {
			FFT(signals->real_input, signals->imag_input, signals->real_output, signals->imag_input, signals->size);
			complex_abs(signals->real_output, signals->imag_output, signals->magnitude, signals->size);
			vTaskDelay(30 / portTICK_RATE_MS);
	}

	vTaskDelay(portMAX_DELAY);
}

void setupFFT(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals) {
	STM32_AudioRec_Init(SAMPLE_RATE_44100, DEFAULT_IN_BIT_RESOLUTION, DEFAULT_IN_CHANNEL_NBR);
	STM32_AudioRec_Start((uint8_t*)audio_buffer_1, FFT_LENGTH);
  	xTaskCreate(testFFTTask, "fft", 1000, &signals, uxPriority, NULL);
}
