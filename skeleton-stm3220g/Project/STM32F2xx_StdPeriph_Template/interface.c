#include "interface.h"
#include "LCD.h"
#include "semphr.h"
#include <string.h>
#include "touch.h"
#define BUTTON_PAD_X 8

sFONT *Currentfont;


void registerButton(uint16_t x, uint16_t y, char *name, void (*func_call)(void)) {
	Button_t button;
	button.name = name;
	button.x = x; 
	button.y = y; 
	button.width = BUTTON_PAD_X*2 + strlen(name) * Currentfont->Width; 
	button.height = 2*Currentfont->Height;
	
	uint16_t left = button.x;
	uint16_t right = button.x + button.width;
	uint16_t upper = button.y;
	uint16_t lower = button.y + button.height;
	
	registerTCallback(left, right, lower, upper, button, func_call);
}

/* Write anywhere on the screen
PRE : row is line number , x is the column with origin at top right,
			direction is either veritcal or horizontal
*/
void LCD_write(uint16_t x, uint16_t y, char* ptr, uint8_t direction){
	
	while(*ptr != '\0'){
	  LCD_DisplayChar(y, WIDTH - x, (uint8_t)*ptr);
		ptr++;
		if (direction == LCD_DIR_VERTICAL) 
			y += Currentfont->Height;
		else
			x = x + (Currentfont->Width);
	}
}

/* Drawing buttons < and >  on Line0 
   Write inside the button before drawing buttons.
	 Or rectangles side(s) get removed.	
	 		
*/
void drawButton(Button_t *button){
	uint16_t W, H; 
	
	LCD_write( button->x + BUTTON_PAD_X, button->y + Currentfont->Height/2, button->name, Horizontal); 
	LCD_drawRect( button->x, button->y, button->width, button->height);
}


void writeTitle (char* ptr){
	LCD_write( 1, WIDTH - 40, ptr, Horizontal); 
}

void LCD_drawNonUpdateElements(void) {
	LCD_drawLine(0, 52, WIDTH, Horizontal); // +5 to get lagom
}

// Draw Title
void LCD_drawUpdateElements(void){
	
	// writeTitle ("Visualiser"); 
}

void drawAllButtons() {
	TCallback *callback = getCallbacks();
	uint8_t callbacksNum = getCallbackNum();
	for (int i = 0; i < callbacksNum; i++) {
		drawButton(&callback[i].button);
	}
}

// Initialising graphs 

void InitGraphs(graph_type_t *graphs, uint8_t *graph_index) {
  *graph_index = 0;
	graphs[0].graph_func = draw_simple_white_graph;
	graphs[0].name = "White      ";
	graphs[1].graph_func = draw_simple_rainbow_graph;
	graphs[1].name = "Rainbow    ";
	graphs[2].graph_func = draw_block_rainbow_graph;
	graphs[2].name = "JAY        ";
  graphs[3].graph_func = draw_block_mirror_rainbow_graph;
	graphs[3].name = "Mirrored   ";
}





// Run interface
void interfaceTask(void* params){
	uint8_t prevIndex = NUM_OF_GRAPHS; 
	context_t *ctx = (context_t *) params;
	Currentfont = LCD_GetFont();
	// Creating/registering buttons
	registerButton(1, 1, "<", leftToUp);
	registerButton(WIDTH - 90, 1, ">", rightToDown);
	registerButton(WIDTH - 50, 1, "EQ", toggleOrange);
	
	xSemaphoreTake(ctx->lcd_lock, portMAX_DELAY);
	LCD_setColors(White, Black);
	LCD_drawNonUpdateElements();
	drawAllButtons();
	xSemaphoreGive(ctx->lcd_lock);
	for(;;){
		if (prevIndex != *(ctx->graph_index)) {
			xSemaphoreTake(ctx->lcd_lock, portMAX_DELAY);
			LCD_setColors(White, Black);
			LCD_write(40, 12, ctx->graphs[*(ctx->graph_index)].name, Horizontal);
			
			xSemaphoreGive(ctx->lcd_lock);
			prevIndex = *(ctx->graph_index);
		}
		STM_EVAL_LEDToggle(LED1);
		vTaskDelay(400 / portTICK_RATE_MS);
	}	
}

// Interface Setup. To be called in the main.c
void setupInterface(context_t *ctx) {
	InitGraphs(ctx->graphs, ctx->graph_index);
	xTaskCreate( interfaceTask, "interface", 100, ctx, 1, NULL);
}

