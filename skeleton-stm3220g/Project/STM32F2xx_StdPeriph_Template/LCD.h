/*----------------------------------------------------------------------------
 * Name:    LCD.h
 * Purpose: API ll Graphic LCD (320x240 pixels) on stm3210 and stm3220
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

#ifndef _LCD_H
#define _LCD_H
#include "stm322xg_eval_lcd.h"
/*------------------------------------------------------------------------------
  color coding.
  LCD is coded:   15..11 red, 10..5 green, 4..0 blue  (unsigned short)  LCD_R5, LCD_G6, LCD_B5   
  original coding: 17..12 red, 11..6 green, 5..0 blue                    ORG_R6,  ORG_G6,  ORG_B6

  ORG_R1..5 = LCD_R0..4,  ORG_R0 = LCD_R4
  ORG_G0..5 = LCD_G0..5,
  ORG_B1..5 = LCD_B0..4,  ORG_B0 = LCD_B4
 *------------------------------------------------------------------------------*/
                            
/**
 * The underlying coordinate system is  a bit cumbersome:
 * the origin (0, 0) is in the upper right corner
 * this API maps (0, 0) to top left corner.
 */
#define WIDTH 320 
#define HEIGHT 240

extern void LCD_init           (void);
extern void LCD_clear          (unsigned short color);
extern void LCD_setTextColor   (unsigned short color);
extern void LCD_setBackColor   (unsigned short color);
extern void LCD_setColors      (unsigned short text, unsigned short back);
extern void LCD_getColors      (unsigned short *text, unsigned short *back);
extern void LCD_displayChar    (unsigned int ln, unsigned int col, unsigned char  c);
extern void LCD_displayStringLn(unsigned int ln, unsigned char *s);
extern void LCD_clearLn        (unsigned int ln);
extern void LCD_putPixel       (unsigned int x, unsigned int y);
extern void LCD_drawLine(uint16_t x, uint16_t y, uint16_t Length, uint16_t Direction);
extern void LCD_drawRect(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height);
extern void LCD_fillRect(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height);

#endif /* _LCD_H */
