#include "arm_math.h"
#include "stm322xg_eval_ioe.h"
#include "stm32f2xx_adc.h"
#include "fft.h"

#define FFT_LENGTH 256 /* number of frequency bins/segments */


static void initADC() {
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 1,
	ADC_SampleTime_480Cycles);
}

static void transformTask(void *params) {
	static arm_rfft_instance_q15 fft_instance;
	arm_status status;
	
	uint32_t inverse_fft = 0; /* use forward FFT */
	uint32_t bit_reverse_flag = 1; /* use normal bit order for output */
	
	static q15_t output[FFT_LENGTH*2]; //has to be twice FFT size because of symmetry
	
	for (;;) {
		status = arm_rfft_init_q15(&fft_instance, FFT_LENGTH, inverse_fft, bit_reverse_flag);
		printf("FFT init status: %d\n", status);
		
		arm_rfft_q15(&fft_instance, (q15_t*)WAVES[i].data, output); /* perform FFT */
		
		arm_abs_q15(output, output, FFT_SIZE);
		
	}
}

void setupFFT(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(transformTask, "fft", 100, NULL, uxPriority, NULL);
}