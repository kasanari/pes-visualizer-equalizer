/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "semphr.h"
#include "LCD.h"
#include "stm322xg_eval_ioe.h"
#include "interface.h"
#include "touch.h"
#include "fft.h"
#include "color.h"
#include "graph.h"
#include "global.h"
#include "fft_test.h"

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

xSemaphoreHandle lcdLock; 

void setupHW() {
		/*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */ 
		/*Initialize the LEDs*/
		STM_EVAL_LEDInit(LED1);
		STM_EVAL_LEDInit(LED2);
		STM_EVAL_LEDInit(LED3);
		STM_EVAL_LEDInit(LED4);

		/* SysTick end of count event each 1ms */
		RCC_GetClocksFreq(&RCC_Clocks);
		SysTick_Config(RCC_Clocks.HCLK_Frequency);


}
  /* LCD Module init */
static void initDisplay () {
	LCD_init();
	LCD_setTextColor(White);
	LCD_setBackColor(Black);
  LCD_clear(Black);
	LCD_SetFont(&Font16x24);
  lcdLock = xSemaphoreCreateMutex();
}

xQueueHandle printQueue;

/* Retarget printing to the serial port 1 */

int fputc(int ch, FILE *f) {
  unsigned char c = ch;
  xQueueSend(printQueue, &c, 0);
  return ch;
}

/*-----------------------------------------------------------*/


const int zeros[FFT_LENGTH];	

FLOAT_TYPE fft_out_real[FFT_LENGTH];
FLOAT_TYPE fft_out_imag[FFT_LENGTH];
FLOAT_TYPE fft_magnitude[FFT_LENGTH/2];

xSemaphoreHandle fft_done;
xSemaphoreHandle graph_done;
	
FFT_signals_t signals;

uint8_t graph_index;	
	
graph_type_t graphs[NUM_OF_GRAPHS];
context_t ctx;

int main (void) {
  setupHW();
	IOE_Config(); /*Needed for the touch screen functionality*/
	initDisplay();
	/* Attempt to create a semaphore. */
  fft_done = xSemaphoreCreateBinary();
	graph_done = xSemaphoreCreateBinary();
	//xSemaphoreTake(fft_done, portMAX_DELAY);
	
  signals.imag_input = (FLOAT_TYPE*)zeros; 
	signals.real_output = fft_out_real; 
	signals.imag_output = fft_out_imag; 
	signals.magnitude = fft_magnitude;
	signals.fft_done_lock = fft_done;
	signals.graph_done_lock = graph_done;
	
	ctx.lcd_lock = lcdLock;
	ctx.graphs = graphs; 
	ctx.graph_index = &graph_index;
	ctx.signals = &signals;
	printQueue = xQueueCreate(128, 1);
	
	setupFFT(1, &signals);
	plot_fft_graph(&ctx);
	
	setupInterface(&ctx);
	setupTouch(&graph_index);
	printf("Setup complete ");  // this is redirected to the display

	
  vTaskStartScheduler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

