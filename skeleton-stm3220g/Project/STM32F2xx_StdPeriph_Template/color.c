
#include "color.h"
#include <stdio.h>

unsigned short getColorFromHSL(unsigned short hue, unsigned short saturation, unsigned short lightness) {
	int R, G, B;
	int m;
	int second;
	int hue_limited = hue % 360;
	
	int chroma = (100 - abs(2*lightness - 100)) * saturation;
	chroma /= 100;
	
	// Modulo
	second = hue_limited - (hue_limited/120)*120;
	// change scaling from 0:120 to 0:200
	second = (10*second)/6;
	second = abs(second - 100);
	second = chroma * (100 - second);
	second /= 100;
	
	if (hue_limited < 60) {
		R = chroma; 
		G = second; 
		B = 0;
	} else if (hue_limited < 120) {
		R = second; 
		G = chroma; 
		B = 0;
	} else if (hue_limited < 180) {
		R = 0; 
		G = chroma; 
		B = second;
	} else if (hue_limited < 240) {
		R = 0; 
		G = second; 
		B = chroma;
	} else if (hue_limited < 300) {
		R = second; 
		G = 0; 
		B = chroma;
	} else if (hue_limited < 360) {
		R = chroma; 
		G = 0; 
		B = second;
	}
	
	m = lightness - (chroma/2);
	
	R += m;
	G += m;
	B += m;
	
	// Turns values from 0-100 -> 16-bit color
	R = (R * 31) / 100;
	G = (G * 63) / 100;
	B = (B * 31) / 100;
	return GET_COLOR_16BIT(R, G, B);
}

unsigned short getColorFromHSL_float(unsigned short hue, float saturation, float lightness) {
	float R, G, B;
	float m;
	unsigned short Ri, Gi, Bi;
	int hue_limited = (float) (hue % 360);
	
	float chroma = (1 - fabs(2*lightness - 1)) * saturation;
	
	float hue_prime = ((float) hue_limited) / 60;
	float second = fmod(hue_prime, 2);
	second = fabs(second - 1);
	second = (1 - second);
	second = (chroma * second);
	
	if (hue_limited < 60) {
		R = chroma; 
		G = second; 
		B = 0;
	} else if (hue_limited < 120) {
		R = second; 
		G = chroma; 
		B = 0;
	} else if (hue_limited < 180) {
		R = 0; 
		G = chroma; 
		B = second;
	} else if (hue_limited < 240) {
		R = 0; 
		G = second; 
		B = chroma;
	} else if (hue_limited < 300) {
		R = second; 
		G = 0; 
		B = chroma;
	} else if (hue_limited < 360) {
		R = chroma; 
		G = 0; 
		B = second;
	}
	
	m = lightness - (chroma/2);
	
	R += m;
	G += m;
	B += m;
	
	printf("//RGB: %.3f, %.3f, %.3f", R,G,B);
	
	// Turns values from 0-100 -> 16-bit color
	Ri = (int) (R * 31);
	Gi = (int) (G * 63);
	Bi = (int) (B * 31);
	
	printf("//rgb: %d, %d, %d", Ri, Gi, Bi);
	
	return GET_COLOR_16BIT(Ri, Gi, Bi);
}

