#include "stm32f2xx_adc.h"
#include "FreeRTOS.h"
#include "fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "global.h"

static float inverse_real[FFT_LENGTH];
static float inverse_imag[FFT_LENGTH];
static float out_i[FFT_LENGTH];
static float out_r[FFT_LENGTH];
static float mag[FFT_LENGTH];
static FFT_signals_t test_signals;

void test_reset() {
	test_signals.real_input = NULL;
	test_signals.imag_input = NULL;
	test_signals.real_output = NULL;
	test_signals.imag_output = NULL;
	test_signals.magnitude = NULL;
	for (int i = 0; i < FFT_LENGTH; i++) {
		out_i[i] = 0;
		out_r[i] = 0;
		mag[i] = 0;
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
	for (int i = 0; i < 5; i++) {
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

void fft_test(FFT_signals_t *signals, float *expected_real_output, float* expected_imag_ouput, float* expected_magnitude, bool* test_results, int n) {
	
	// Check that the output is what we expect
	FFT(signals->real_input, signals->imag_input, signals->real_output, signals->imag_output, n);
	
	test_results[0] = check(buffer_compare(signals->real_output, expected_real_output, n), "test_real");
	test_results[1] = check(buffer_compare(signals->imag_output, expected_imag_ouput, n), "test_imag");


	// Check that the inverse is equal to the original input
	inverse_FFT(signals->real_output, signals->imag_output, inverse_real, inverse_imag, n);

	test_results[2] = check(buffer_compare(signals->real_input, inverse_real, n), "test_inverse_real");
	test_results[3] = check(buffer_compare(signals->imag_input, inverse_imag, n), "test_inverse_imag");
	
	complex_abs(signals->real_output, signals->imag_output, signals->magnitude, n);
	
	test_results[4] = check(buffer_compare(signals->magnitude, expected_magnitude, n), "test_magnitude");
	
}

// Impulse
bool test_1() {
		float input_real[4] = {1, 0, 0, 0};
		float input_imag[4] = {0, 0, 0, 0};
		float expected_output_real[4] = {1, 1, 1, 1};
		float expected_output_imag[4] = {0, 0, 0, 0};
		float expected_magnitude[4] = {1, 1, 1, 1};
		bool test_results[5] = {0, 0, 0, 0, 0};

		if (FFT_Init(4)) {

		test_signals.real_input = input_real;
		test_signals.imag_input = input_imag;
		test_signals.real_output = out_r;
		test_signals.imag_output = out_i;
		test_signals.magnitude = mag;
		
		fft_test(&test_signals, expected_output_real, expected_output_imag, expected_magnitude, test_results, 4);
		test_reset();
		return check_test_result(test_results);
	} else {
		return false;
	}
}
	
// sin(x)
bool test_2() {
	float input_real[FFT_LENGTH] = {0.000000, 0.099568, 0.198146, 0.294755, 0.388435, 0.478254, 0.563320, 0.642788, 0.715867, 0.781831, 0.840026, 0.889872, 0.930874, 0.962624, 0.984808, 0.997204, 0.999689, 0.992239, 0.974928, 0.947927, 0.911506, 0.866025, 0.811938, 0.749781, 0.680173, 0.603804, 0.521435, 0.433884, 0.342020, 0.246757, 0.149042, 0.049846, -0.049846, -0.149042, -0.246757, -0.342020, -0.433884, -0.521435, -0.603804, -0.680173, -0.749781, -0.811938, -0.866025, -0.911506, -0.947927, -0.974928, -0.992239, -0.999689, -0.997204, -0.984808, -0.962624, -0.930874, -0.889872, -0.840026, -0.781831, -0.715867, -0.642788, -0.563320, -0.478254, -0.388435, -0.294755, -0.198146, -0.099568, -0.000000};
	float input_imag[FFT_LENGTH] = {0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000};
	float expected_output_real[FFT_LENGTH] = {-0.000000, 1.557136, -0.067150, -0.056279, -0.053261, -0.051971, -0.051296, -0.050898, -0.050643, -0.050470, -0.050347, -0.050256, -0.050188, -0.050135, -0.050093, -0.050060, -0.050033, -0.050010, -0.049992, -0.049976, -0.049963, -0.049953, -0.049943, -0.049936, -0.049929, -0.049924, -0.049919, -0.049916, -0.049913, -0.049911, -0.049909, -0.049908, -0.049908, -0.049908, -0.049909, -0.049911, -0.049913, -0.049916, -0.049919, -0.049924, -0.049929, -0.049936, -0.049943, -0.049953, -0.049963, -0.049976, -0.049992, -0.050010, -0.050033, -0.050060, -0.050093, -0.050135, -0.050188, -0.050256, -0.050347, -0.050470, -0.050643, -0.050898, -0.051296, -0.051971, -0.053261, -0.056279, -0.067150, 1.557136};
	float expected_output_imag[FFT_LENGTH] = {0.000000, -31.696241, 0.681785, 0.379400, 0.267760, 0.207480, 0.169102, 0.142251, 0.122263, 0.106710, 0.094192, 0.083848, 0.075112, 0.067599, 0.061039, 0.055232, 0.050033, 0.045327, 0.041027, 0.037065, 0.033385, 0.029940, 0.026695, 0.023618, 0.020681, 0.017863, 0.015143, 0.012503, 0.009928, 0.007404, 0.004916, 0.002452, 0.000000, -0.002452, -0.004916, -0.007404, -0.009928, -0.012503, -0.015143, -0.017863, -0.020681, -0.023618, -0.026695, -0.029940, -0.033385, -0.037065, -0.041027, -0.045327, -0.050033, -0.055232, -0.061039, -0.067599, -0.075112, -0.083848, -0.094192, -0.106710, -0.122263, -0.142251, -0.169102, -0.207480, -0.267760, -0.379400, -0.681785, 31.696241};
	float expected_magnitude[64] = {0.000000, 31.734467, 0.685084, 0.383551, 0.273006, 0.213890, 0.176711, 0.151083, 0.132337, 0.118043, 0.106804, 0.097755, 0.090336, 0.084162, 0.078962, 0.074543, 0.070757, 0.067495, 0.064672, 0.062221, 0.060091, 0.058238, 0.056630, 0.055239, 0.054043, 0.053023, 0.052166, 0.051458, 0.050891, 0.050457, 0.050151, 0.049968, 0.049908, 0.049968, 0.050151, 0.050457, 0.050891, 0.051458, 0.052166, 0.053023, 0.054043, 0.055239, 0.056630, 0.058238, 0.060091, 0.062221, 0.064672, 0.067495, 0.070757, 0.074543, 0.078962, 0.084162, 0.090336, 0.097755, 0.106804, 0.118043, 0.132337, 0.151083, 0.176711, 0.213890, 0.273006, 0.383551, 0.685084, 31.734467};
	bool test_results[5] = {0, 0, 0, 0, 0};
	
	if(FFT_Init(FFT_LENGTH)) {			
		
		test_signals.real_input = input_real;
		test_signals.imag_input = input_imag;
		test_signals.real_output = out_r;
		test_signals.imag_output = out_i;
		test_signals.magnitude = mag;		

		fft_test(&test_signals, expected_output_real, expected_output_imag, expected_magnitude, test_results, FFT_LENGTH);
		test_reset();
		return check_test_result(test_results);
	} else {
		return false;
	}
}

//exp(2*pi*x*1i)
bool test_3() {
		float input_real[FFT_LENGTH] = {1.000000, 0.810002, 0.312205, -0.304228, -0.805056, -0.999965, -0.814890, -0.320160, 0.296230, 0.800054, 0.999859, 0.819721, 0.328092, -0.288211, -0.794995, -0.999684, -0.824495, -0.336001, 0.280172, 0.789881, 0.999438, 0.829211, 0.343887, -0.272113, -0.784711, -0.999121, -0.833868, -0.351748, 0.264035, 0.779486, 0.998735, 0.838467, 0.359585, -0.255939, -0.774206, -0.998278, -0.843007, -0.367396, 0.247824, 0.768872, 0.997751, 0.847488, 0.375181, -0.239692, -0.763484, -0.997154, -0.851909, -0.382941, 0.231544, 0.758042, 0.996487, 0.856270, 0.390673, -0.223379, -0.752547, -0.995749, -0.860571, -0.398377, 0.215198, 0.746999, 0.994942, 0.864811, 0.406054, -0.207002};
		float input_imag[FFT_LENGTH] = {0.000000, 0.586428, 0.950015, 0.952599, 0.593199, 0.008385, -0.579615, -0.947364, -0.955117, -0.599928, -0.016769, 0.572762, 0.944646, 0.957567, 0.606615, 0.025152, -0.565869, -0.941862, -0.959950, -0.613260, -0.033533, 0.558936, 0.939011, 0.962265, 0.619861, 0.041912, -0.551963, -0.936095, -0.964513, -0.626419, -0.050288, 0.544952, 0.933112, 0.966693, 0.632933, 0.058661, -0.537903, -0.930065, -0.968805, -0.639402, -0.067029, 0.530815, 0.926951, 0.970849, 0.645827, 0.075393, -0.523691, -0.923773, -0.972825, -0.652206, -0.083751, 0.516529, 0.920530, 0.974732, 0.658539, 0.092103, -0.509331, -0.917222, -0.976570, -0.664826, -0.100449, 0.502098, 0.913849, 0.978341};
		float expected_output_real[FFT_LENGTH] = {1.906317, 2.110843, 2.406120, 2.872661, 3.726112, 5.805854, 18.719448, -10.202924, -3.347429, -1.726765, -0.999162, -0.584329, -0.315204, -0.125710, 0.015532, 0.125341, 0.213541, 0.286258, 0.347511, 0.400048, 0.445811, 0.486213, 0.522309, 0.554901, 0.584613, 0.611936, 0.637263, 0.660917, 0.683161, 0.704216, 0.724269, 0.743480, 0.761989, 0.779917, 0.797376, 0.814463, 0.831271, 0.847887, 0.864391, 0.880866, 0.897389, 0.914042, 0.930908, 0.948074, 0.965633, 0.983687, 1.002349, 1.021744, 1.042016, 1.063332, 1.085884, 1.109904, 1.135668, 1.163512, 1.193849, 1.227201, 1.264225, 1.305777, 1.352983, 1.407362, 1.471016, 1.546938, 1.639546, 1.755649};
		float expected_output_imag[FFT_LENGTH] = {2.351868, 2.882607, 3.648845, 4.859505, 7.074188, 12.471064, 45.981498, -29.071291, -11.281463, -7.075883, -5.187772, -4.111291, -3.412919, -2.921189, -2.554667, -2.269716, -2.040839, -1.852141, -1.693191, -1.556860, -1.438106, -1.333263, -1.239595, -1.155019, -1.077918, -1.007017, -0.941292, -0.879911, -0.822189, -0.767552, -0.715515, -0.665662, -0.617633, -0.571109, -0.525805, -0.481464, -0.437847, -0.394730, -0.351901, -0.309151, -0.266273, -0.223059, -0.179292, -0.134748, -0.089182, -0.042332, 0.006094, 0.056424, 0.109030, 0.164344, 0.222868, 0.285199, 0.352055, 0.424308, 0.503034, 0.589579, 0.685657, 0.793483, 0.915982, 1.057095, 1.222274, 1.419289, 1.659605, 1.960888};
		float expected_magnitude[FFT_LENGTH] = {3.027429, 3.572826, 4.370754, 5.645084, 7.995502, 13.756285, 49.645905, 30.809733, 11.767612, 7.283532, 5.283115, 4.152608, 3.427444, 2.923892, 2.554714, 2.273174, 2.051981, 1.874132, 1.728484, 1.607436, 1.505622, 1.419153, 1.345140, 1.281399, 1.226246, 1.178367, 1.136721, 1.100480, 1.068973, 1.041660, 1.018100, 0.997932, 0.980866, 0.966662, 0.955133, 0.946127, 0.939533, 0.935267, 0.933277, 0.933541, 0.936060, 0.940866, 0.948016, 0.957601, 0.969742, 0.984597, 1.002367, 1.023300, 1.047705, 1.075957, 1.108519, 1.145961, 1.188985, 1.238466, 1.295500, 1.361479, 1.438190, 1.527963, 1.633887, 1.760147, 1.912549, 2.099381, 2.332895, 2.631993};
		bool test_results[5] = {0, 0, 0, 0, 0};
	
		if (FFT_Init(FFT_LENGTH)) {

		test_signals.real_input = input_real;
		test_signals.imag_input = input_imag;
		test_signals.real_output = out_r;
		test_signals.imag_output = out_i;
		test_signals.magnitude = mag;
		
		fft_test(&test_signals, expected_output_real, expected_output_imag, expected_magnitude, test_results, FFT_LENGTH);
		test_reset();
		return check_test_result(test_results);
	} else {
		return false;
	}
}

static void testFFTTask(void *params) {
	bool test1_result;
	bool test2_result;
	bool test3_result;
	
	test1_result = test_1();
	test2_result = test_2();
	test3_result = test_3();
	
	if (test1_result == false) {
		printf("Put breakpoint here to check if test1 failed.");
	}
	
	if (test2_result == false) {
		printf("Put breakpoint here to check if test2 failed.");
	}
	
	if (test3_result == false) {
		printf("Put breakpoint here to check if test3 failed.");
	}

	vTaskDelay(portMAX_DELAY);
}

void setupFFTTest(unsigned portBASE_TYPE uxPriority, FFT_signals_t *signals) {
	BaseType_t xReturned;
	xReturned = xTaskCreate(testFFTTask, "fft_test", 1000, signals, uxPriority, NULL);
	if( xReturned == pdPASS ) {
		printf("FFT test task created!");
	} else {
		printf("FFT test task could not be created!");
	}
}
