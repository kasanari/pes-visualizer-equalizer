#include "arm_math.h"
#include "stm32f2xx_adc.h"
#include "stm32_audio.h"
#include "stm322xg_audio_recorder.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "fft.h"
#include <stdio.h>
#include <stdlib.h>

#define FFT_LENGTH 256 				/* number of frequency bins/segments */

#define DEFAULT_VOLUME  70    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values      */                                        
																									
xSemaphoreHandle buffer_switch_sem;

uint16_t* audio_buffer_1;
uint16_t* audio_buffer_2;

uint8_t	 buffer_select = 1;

uint32_t STM32_AudioRec_Update(uint8_t** pbuf, uint32_t* pSize) {
	portBASE_TYPE higherPrio;
	*pSize = FFT_LENGTH;
  *pbuf = (buffer_select) ? (uint8_t*)audio_buffer_1 : (uint8_t*)audio_buffer_2;
	/* Toggle the buffer select */
  buffer_select = (buffer_select == 0) ? 1 : 0;
	
	//xSemaphoreGiveFromISR(buffer_switch_sem, &higherPrio);
	//vPortYieldFromISR();
	
	return 0;
}

static void transformTask(void *params) {
	static arm_rfft_instance_q15 fft_instance;
	arm_status status;
	
	uint32_t inverse_fft = 0; /* use forward FFT */
	uint32_t bit_reverse_flag = 1; /* use normal bit order for output */
	
	static q15_t output[FFT_LENGTH*2]; //has to be twice FFT size because of symmetry
	
	audio_buffer_1 = calloc(FFT_LENGTH, sizeof(uint16_t));
	audio_buffer_2 = calloc(FFT_LENGTH, sizeof(uint16_t));
	
	STM32_AudioRec_Init(SAMPLE_RATE_44100, DEFAULT_IN_BIT_RESOLUTION, DEFAULT_IN_CHANNEL_NBR);
	STM32_AudioRec_Start((uint8_t*)audio_buffer_1, FFT_LENGTH);
	
	status = arm_rfft_init_q15(&fft_instance, FFT_LENGTH, inverse_fft, bit_reverse_flag);
	printf("FFT init status: %d\n", status);
	
	for (;;) {

		arm_rfft_q15(&fft_instance, (q15_t*)audio_buffer_1, output); /* perform FFT */
		
		arm_abs_q15(output, output, FFT_LENGTH);
		vTaskDelay(300 / portTICK_RATE_MS);
		
	}
}

void setupFFT(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(transformTask, "fft", 100, NULL, uxPriority, NULL);
}