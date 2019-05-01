/*----------------------------------------------------------------------------
 * Name:    LCD.c
 * Purpose: API ll Graphic LCD (320x240 pixels) on stm3220
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * The API is developed as part of the Programming Embedded Systems course 
 * at Uppsala University.
 * This software is supplied "AS IS" without warranties of any kind.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include "LCD.h"

void LCD_init(void){
	STM322xG_LCD_Init();
}

void LCD_clear(unsigned short color){
	LCD_Clear(color);
}
void LCD_setTextColor(unsigned short color){
	LCD_SetTextColor(color);
}

void LCD_setBackColor(unsigned short color){
  LCD_SetBackColor(color);
}

void LCD_setColors(unsigned short text, unsigned short back){
  LCD_SetColors(text, back);
}

void LCD_getColors(unsigned short *text, unsigned short *back){
	LCD_GetColors(text, back);	
}
void LCD_displayChar(unsigned int ln, unsigned int col, unsigned char  c){
  LCD_DisplayChar(ln, WIDTH - col, c);
}
void LCD_displayStringLn(unsigned int ln, unsigned char *s){
	LCD_DisplayStringLine(ln, s);
}
/*TODO*/
void LCD_bitmap(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap);
void LCD_clearLn(unsigned int ln){
	LCD_ClearLine(ln);
}
void LCD_putPixel(unsigned int x, unsigned int y){
	PutPixel(y, WIDTH - x);
}
void LCD_drawLine(uint16_t x, uint16_t y, uint16_t length, uint16_t direction){
	LCD_DrawLine(y, WIDTH - x, length, direction);
}
void LCD_drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	LCD_DrawRect(y, WIDTH - x, height, width);
}
void LCD_fillRect(uint16_t x, uint16_t y, uint16_t height, uint16_t width){
	LCD_DrawFullRect(y, WIDTH - x, height, width);
}
