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
#include "TS.h"
#include "stm322xg_eval_ioe.h"
#include "fft.h"
#include "color.h"
#include "graph.h"

/* Testing variables ------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void setupHW (void){
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

xSemaphoreHandle lcdLock;


static void initDisplay () {
  /* LCD Module init */
  lcdLock = xSemaphoreCreateMutex();

  LCD_init();
	LCD_SetFont(&Font16x24);
  LCD_clear(Black);
	LCD_setBackColor(Black);
  LCD_setTextColor(Red);
}


/*
static void colorTask(void *params) { 
	int xi, yi;
	int step_size = 10;
	// unsigned short green = 0;
	hue = 0;
	lightness = 0;
	
	for (;;) {
		xSemaphoreTake(lcdLock, portMAX_DELAY);
		for (xi = 0; xi < WIDTH; xi += step_size) {
			hue = (360* xi)/WIDTH;
			for (yi = 0; yi < HEIGHT; yi += step_size) {
				lightness = (float)yi/HEIGHT;
				color = getColorFromHSL_float(hue, 1.0, lightness);
				// color = getColorFromHSL(hue, 100, (unsigned short) 100*lightness);
				// color = GET_COLOR_16BIT(0, green, 0);
				LCD_setColors(color,color);
				LCD_fillRect( xi, yi, step_size, step_size);
				LCD_setColors(Blue, White);	
			}
			// green = (63 * xi) / WIDTH;
		}
		xSemaphoreGive(lcdLock);
	
		hue = (hue > 360) ? 0 : hue;
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}*/

/*-----------------------------------------------------------*/

xQueueHandle printQueue;


/* Retarget printing to the serial port 1 */

int fputc(int ch, FILE *f) {
  unsigned char c = ch;
  xQueueSend(printQueue, &c, 0);
  return ch;
}

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/




/*-----------------------------------------------------------*/
/*
static void testingGraphTask(void *params) {
	int i = 0;
	int function_index = 0;
	drawGraphFunction f;
	TS_STATE *ts_state;
	int draw_change = 1;
	u8 pressed = 0;
	
	for (;;) {
		ts_state = IOE_TS_GetState();
		
		if (pressed) {
			if (!ts_state->TouchDetected) {
				pressed = 0;
			}
		} else if (ts_state->TouchDetected) {
			pressed = 1;
		}
		
		if (pressed) {
			function_index = (function_index+1)%4;
			draw_change = 1;
		}
		
		switch (function_index) {
			case(0):
				f = draw_simple_white_graph;
			break;
			case(1):
				f = draw_simple_rainbow_graph;
			break;
			case(2):
				f = draw_block_rainbow_graph;
			break;
			case(3):
				f = draw_block_mirror_rainbow_graph;
			break;
		}
		
		i = (i+1)%100;
		xSemaphoreTake(lcdLock, portMAX_DELAY);
		if (draw_change) {
			graph_clear_all();
			draw_change = 0;
		}
		run_graph(f, &sine[i]);
		xSemaphoreGive(lcdLock);
		vTaskDelay(30 / portTICK_RATE_MS);
	}
}*/

/*-----------------------------------------------------------*/
int main (void){
	
  setupHW();
	IOE_Config(); /*Needed for the touch screen functionality*/
	
	printQueue = xQueueCreate(128, 1);
	
	initDisplay();
	

	//setupFFT(1);
	// setup_graph(10, 10, WIDTH-10, HEIGHT-10, 32, 256, sine);
	setup_default_graph();
	
	// xTaskCreate(lcdTask, "lcd", 100, NULL, 1, NULL);
  // xTaskCreate(printTask, "print", 100, NULL, 1, NULL);
  // xTaskCreate(ledTask, "led", 100, NULL, 1, NULL);
	// xTaskCreate(touchScreenTask, "touchScreen", 100, NULL, 1, NULL);
	// xTaskCreate(menuTask, "menu", 100, NULL, 1, NULL);
	// xTaskCreate(viewportTask, "viewport", 100, NULL, 1, NULL);
  // xTaskCreate(highlightButtonsTask, "highlighter", 100, NULL, 1, NULL);
	// xTaskCreate(testingGraphTask, "graph", 100, NULL, 1, NULL);
	printf("Setup complete ");  // this is redirected to the display

	
  vTaskStartScheduler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

