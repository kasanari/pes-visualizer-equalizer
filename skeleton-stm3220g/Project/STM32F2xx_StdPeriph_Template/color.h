#ifndef _COLOR_H_
#define _COLOR_H_

// for abs
#include <stdlib.h>

//for fabs 
#include <math.h>

/*  Specify the color in 
 *  Number of bits:				R:5 bits, G:6 bits, B 5 bits
 *  Which bits: 					R[15:11], G[10:5] , B[4:0]   
 *  value/argument range: R[0-31] , G[0-63] , B[0-31]
*/
#define GET_COLOR_16BIT(R31,G63,B31) ((R31 << 11) + (G63 << 5) + B31)


/*  Specify the color in hex values
 *  
 *    
 *  
*/
#define GET_COLOR_24BIT(R,G,B) (GET_COLOR_16BIT((R>>3), (G>>2), (B>>3)))


/* Turns values in HSL model to 16-bit RGB usable for the display 
 * @param hue value from 0 to 359, function will function even if hue > 360 
 * @param saturation value from 0 to 100
 * @param lightness value from 0 to 100
 * @Return 16-bit color 
 * PRE-CONDITION: saturation and lightness are within [0,100]  
*/
unsigned short getColorFromHSL(unsigned short hue, unsigned short saturation, unsigned short lightness);


unsigned short getColorFromHSL_float(unsigned short hue, float saturation, float lightness);

#endif
