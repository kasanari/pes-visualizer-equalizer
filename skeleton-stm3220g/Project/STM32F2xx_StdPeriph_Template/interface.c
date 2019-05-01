#include "interface.h"
#include "LCD.h"
#include "semphr.h"
#include <string.h>

//LED - to check if system is alive
xSemaphoreHandle lcd_temp;
sFONT *Currentfont;

void setupHW(void) {
	STM_EVAL_LEDInit(LED1);
	STM_EVAL_LEDInit(LED2);
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
}


// LED - Task
void ledTask(void* params){
	for(; ;){
		STM_EVAL_LEDToggle(LED1);
		vTaskDelay(400 / portTICK_RATE_MS);
	}	
}

void setupLCD(){
	LCD_init();
	lcd_temp = xSemaphoreCreateMutex();
	xSemaphoreTake( lcd_temp, portMAX_DELAY); 
  LCD_clear(Black);
  LCD_setColors(White,Black);
	LCD_SetFont(&Font16x24);
	xSemaphoreGive(lcd_temp);
}


/* Write anywhere on the screen
PRE : row is line number , x is the column with origin at top right,
			direction is either veritcal or horizontal
*/
void LCD_write( uint16_t row, uint16_t x, char* ptr, uint8_t direction){
	Currentfont = LCD_GetFont();
	while(*ptr != '\0'){
	  LCD_DisplayChar( LINE(row), x, (uint8_t)*ptr);
		ptr++;
		if (direction == LCD_DIR_VERTICAL) 
			row++;
		else
			x = x - (Currentfont->Width);
	}
	
}

/* Drawing buttons < and >  on Line0 
   Write inside the button before drawing buttons.
	 Or rectangles side(s) get removed.	
	 		
*/
void drawButton( uint16_t x, uint16_t y, char* ptr , Button_t button){
	uint16_t W, H; 
	W = Currentfont->Width;
	H = Currentfont->Height;	
	LCD_write( 0, WIDTH - x - 2, ptr, Vertical); // -2 to centre the symbol
	LCD_drawRect( x, y, 1.5*W, H);
	button.status = disable;
}


// Draw Title
void LCD_drawTitleBar(char* title){
	Button_t Right, Left;
	xSemaphoreTake( lcd_temp, portMAX_DELAY);
	LCD_drawLine( 0, Line1 + 5, WIDTH,Horizontal);
	drawButton( 1, 1, "<", Left);
	drawButton( WIDTH - 80, 1, ">", Right);
	LCD_drawLine( WIDTH - 50, 0, 30, Vertical);
	LCD_write( 0, WIDTH - 40, "Visualizer", Horizontal); 
	xSemaphoreGive(lcd_temp);	
}

// Interface Setup. To be called in the main.c
void setupInterface(void){
	setupHW();	
	setupLCD();
	LCD_write( 0, 40, "EQ", Horizontal);
	LCD_drawTitleBar("Visualizer");
	xTaskCreate( ledTask, "led", 100, NULL, 1, NULL);
}
