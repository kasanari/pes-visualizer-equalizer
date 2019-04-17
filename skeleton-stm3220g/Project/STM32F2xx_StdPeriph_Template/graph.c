#include "graph.h"


#define MAX_NUM_FREQ_VALUES 32
#define BAR_SPACING 3 
/*
	
(start_x, start_y) ___________________
									|										|
									|				AREA				|
									|				 TO					|
									|   		DRAW 				|
									|				 IN					|
									|___________________|
																			(end_x, end_y)
		
*/
uint16_t start_x, start_y, end_x, end_y, width, height, max_freq_value, step_width, mid_y;
uint8_t num_freq;
uint16_t last_freq_values[MAX_NUM_FREQ_VALUES];
uint8_t first_time;


void setup_graph(
	uint16_t _start_x, 
	uint16_t _start_y, 
	uint16_t _end_x, 
	uint16_t _end_y, 
	uint8_t _num_freq, 
	uint16_t _max_freq_value
) {
	int i;
	start_x = _start_x; 
	start_y = _start_y;
	end_x = _end_x;  
	end_y = _end_y;  
	num_freq = _num_freq;
	max_freq_value = _max_freq_value;
	
	width = end_x - start_x;
	height = end_y - start_y;	
	step_width = width/num_freq;
	mid_y = start_y + height/2;
	first_time = 1;
	
	// Set last frequency array to zero
	for(i = 0; i < MAX_NUM_FREQ_VALUES; i++) {
		last_freq_values[i] = 0;
	}
}

void run_graph(drawGraphFunction f, uint16_t *freq_values) {
	int i;
	if (!first_time) {
		f(freq_values);
	} else {
		first_time = 0;
	}
	for (i = 0; i < num_freq; i++) {
			last_freq_values[i] = freq_values[i];
	}
}

void draw_simple_white_graph(uint16_t *freq_values) {
	uint16_t i;
	int16_t bar_diff, last_bar, bar;
	for (i = 0; i < num_freq; i++) {
		last_bar 	= (height*last_freq_values[i])/	max_freq_value;
		bar 			= (height*freq_values[i])			/	max_freq_value;
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			LCD_setColors(White, White);		
			LCD_fillRect(start_x + i*step_width, end_y - bar, step_width-BAR_SPACING, bar_diff);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(start_x + i*step_width, end_y - last_bar, step_width-BAR_SPACING, -bar_diff);
		}
	}
}

void draw_simple_rainbow_graph(uint16_t *freq_values) {
	uint16_t i;
	unsigned short color;
	int16_t bar_diff, last_bar, bar;
	for (i = 0; i < num_freq; i++) {
		last_bar 	= (height*last_freq_values[i])/	max_freq_value;
		bar 			= (height*freq_values[i])			/	max_freq_value;
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			color = getColorFromHSL((360*bar)/max_freq_value, 100, 50);
			LCD_setColors(color, color);
			LCD_fillRect(start_x + i*step_width, end_y - bar, step_width-BAR_SPACING, bar_diff);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(start_x + i*step_width, end_y - last_bar, step_width-BAR_SPACING, -bar_diff);
		}
	}
}

void draw_block_rainbow_graph(uint16_t *freq_values) {
	uint16_t i;
	unsigned short color;
	int16_t bar_diff, last_bar, bar;
	uint8_t block_height = 10;
	uint8_t block_diff = block_height - 3;
	for (i = 0; i < num_freq; i++) {
		last_bar 	= (height*last_freq_values[i])/	(block_height * max_freq_value);
		bar 			= (height*freq_values[i])			/	(block_height * max_freq_value);
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			color = getColorFromHSL((360*bar*block_height)/max_freq_value, 100, 50);
			LCD_setColors(color, color);
			LCD_fillRect(start_x + i*step_width, end_y - bar*block_height, step_width-BAR_SPACING, bar_diff*block_diff);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(start_x + i*step_width, end_y - last_bar*block_height, step_width-BAR_SPACING, -bar_diff*(block_height));
		}
	}
}

void draw_block_mirror_rainbow_graph(uint16_t *freq_values) {
	uint16_t i;
	unsigned short color;
	int16_t bar_diff, last_bar, bar, mirror_lightness;
	uint8_t block_height = 30;
	uint8_t block_diff = block_height - 10;
	
	for (i = 0; i < num_freq; i++) {
		last_bar 	= (height*last_freq_values[i])/	(block_height * max_freq_value);
		bar 			= (height*freq_values[i])			/	(block_height * max_freq_value);
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			color = getColorFromHSL((360*bar*block_height)/max_freq_value, 100, 50);
			LCD_setColors(color, color);
			LCD_fillRect(start_x + i*step_width, mid_y - bar*block_height/2, step_width-BAR_SPACING, bar_diff*block_diff/2);
			mirror_lightness = 40-6*bar;
			mirror_lightness = mirror_lightness > 0 ? mirror_lightness : 0;
			color = getColorFromHSL((360*bar*block_height)/max_freq_value, 100, mirror_lightness);
			LCD_setColors(color, color);
			LCD_fillRect(start_x + i*step_width, mid_y + (bar-1)*block_height/2, step_width-BAR_SPACING, bar_diff*block_diff/2);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(start_x + i*step_width, mid_y - last_bar*block_height/2, step_width-BAR_SPACING, -bar_diff*block_diff/2);
			LCD_fillRect(start_x + i*step_width, mid_y + (last_bar-1)*block_height/2, step_width-BAR_SPACING, -bar_diff*block_diff/2);
		}
	}
}

