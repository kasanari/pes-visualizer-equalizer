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
  LCD_clear(White);
  LCD_setTextColor(Blue);
  LCD_displayStringLn(Line2, " Programming");
  LCD_displayStringLn(Line3, " Embedded");
  LCD_displayStringLn(Line4, " Systems");
}


static void lcdTask(void *params) {
  unsigned short col1 = Blue, col2 = Red, col3 = Green;
  unsigned short t;

  for (;;) {
    xSemaphoreTake(lcdLock, portMAX_DELAY);
    LCD_setTextColor(col1);
    LCD_displayChar(Line7, 40, '1');
    LCD_setTextColor(col2);
    LCD_displayChar(Line7, 60, '2');
    LCD_setTextColor(col3);
    LCD_displayChar(Line7, 80, '3');
    xSemaphoreGive(lcdLock);
	t = col1; col1 = col2; col2 = col3; col3 = t;
    vTaskDelay(300 / portTICK_RATE_MS);
  }
}


/*-----------------------------------------------------------*/

/**
 * Display stdout on the display
 */

xQueueHandle printQueue;

static void printTask(void *params) {
  unsigned char str[21] = "                    ";
  portTickType lastWakeTime = xTaskGetTickCount();
  int i;

  for (;;) {
    xSemaphoreTake(lcdLock, portMAX_DELAY);
    LCD_setTextColor(Black);
    LCD_displayStringLn(Line9, str);
    xSemaphoreGive(lcdLock);

    for (i = 0; i < 19; ++i)
	  str[i] = str[i+1];

    if (!xQueueReceive(printQueue, str + 19, 0))
	  str[19] = ' ';

	vTaskDelayUntil(&lastWakeTime, 100 / portTICK_RATE_MS);
  }
}

/* Retarget printing to the serial port 1 */
int fputc(int ch, FILE *f) {
  unsigned char c = ch;
  xQueueSend(printQueue, &c, 0);
  return ch;
}

/*-----------------------------------------------------------*/

/**
 * Blink the LEDs to show that we are alive
 */

static void ledTask(void *params) {
	Led_TypeDef leds[LEDn] = {LED1, LED2, LED3, LED4};
  int cnt = 0;

  for (;;) {
    STM_EVAL_LEDToggle(leds[cnt]);
    cnt = (cnt + 1) % sizeof(leds);
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

/*-----------------------------------------------------------*/
void touchScreenTask(void *params) {
  portTickType lastWakeTime = xTaskGetTickCount();
  TS_STATE *ts_state;
  u8 pressed = 0;
  u8 i;

  for (;;) {
    ts_state = IOE_TS_GetState();

	if (pressed) {
	  if (!ts_state->TouchDetected)
	    pressed = 0;
	} else if (ts_state->TouchDetected) {
	  for (i = 0; i < callbackNum; ++i) {
		if (callbacks[i].left  <= ts_state->X &&
		    callbacks[i].right >= ts_state->X &&
		    callbacks[i].lower >= ts_state->Y &&
		    callbacks[i].upper <= ts_state->Y)
		  callbacks[i].callback(ts_state->X, ts_state->Y, ts_state->Z,
		                        callbacks[i].data);
	  }													
	  pressed = 1;
	}

    if (ts_state->TouchDetected) {
	  printf("%d,%d,%d ", ts_state->X, ts_state->Y, ts_state->Z);
	}

	vTaskDelayUntil(&lastWakeTime, 100 / portTICK_RATE_MS);
  }
}
/*-----------------------------------------------------------*/

xQueueHandle buttonQueue;

static void highlightButton(u16 x, u16 y, u16 pressure, void *data) {
  u16 d = (int)data;
  xQueueSend(buttonQueue, &d, 0);
}

static void setupButtons(void) {
  u16 i;
  buttonQueue = xQueueCreate(4, sizeof(u16));
  
	LCD_putPixel(30, 40);
  for (i = 0; i < 3; ++i) {
    LCD_drawRect( 250, 30 + 60*i, 40, 40);
		registerTSCallback(WIDTH - 30 - 40, WIDTH - 30, 
		                    30 + 60*i + 40, 30 + 60*i,
	                     &highlightButton, (void*)i);
  }
}

static void highlightButtonsTask(void *params) {
  u16 d;

  for (;;) {
    xQueueReceive(buttonQueue, &d, portMAX_DELAY);

    xSemaphoreTake(lcdLock, portMAX_DELAY);
    LCD_setColors(Blue,Red);
    LCD_fillRect( 250,30 + 60*d, 40, 40);
    LCD_setColors(Blue, White);	
    xSemaphoreGive(lcdLock);

	vTaskDelay(500 / portTICK_RATE_MS);
		
    xSemaphoreTake(lcdLock, portMAX_DELAY);
		LCD_setColors(Blue, White);	
    LCD_fillRect( 250, 30 + 60*d, 40, 40);
    
    xSemaphoreGive(lcdLock);
  }
}

/*-----------------------------------------------------------*/
int main (void){
	
  setupHW();
	IOE_Config(); /*Needed for the touch screen functionality*/
	
	printQueue = xQueueCreate(128, 1);
	
	initDisplay();
	setupButtons();
	
	xTaskCreate(lcdTask, "lcd", 100, NULL, 1, NULL);
  xTaskCreate(printTask, "print", 100, NULL, 1, NULL);
  xTaskCreate(ledTask, "led", 100, NULL, 1, NULL);
  xTaskCreate(touchScreenTask, "touchScreen", 100, NULL, 1, NULL);
  xTaskCreate(highlightButtonsTask, "highlighter", 100, NULL, 1, NULL);
	
	printf("Setup complete ");  // this is redirected to the display

	
  vTaskStartScheduler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

