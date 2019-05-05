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


xSemaphoreHandle lcdLock;


/**
 * Display stdout on the display
 */

xQueueHandle printQueue;

static void printTask(void *params) {
  unsigned char str[21] = "                    ";
  portTickType lastWakeTime = xTaskGetTickCount();
	int i;
	lcdLock = xSemaphoreCreateMutex();
  

  for (;;) {
    xSemaphoreTake(lcdLock, portMAX_DELAY);
    LCD_setTextColor(White);
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

int main (void){
	

	IOE_Config(); /*Needed for the touch screen functionality*/
	
	printQueue = xQueueCreate(128, 1);
	
	setupInterface();
	setupTouch();
  xTaskCreate(printTask, "print", 100, NULL, 1, NULL);
	printf("Setup complete ");  // this is redirected to the display

	
  vTaskStartScheduler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

