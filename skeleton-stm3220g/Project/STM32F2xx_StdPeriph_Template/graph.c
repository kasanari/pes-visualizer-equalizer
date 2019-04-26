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
uint16_t sine[200];
uint16_t start_x, start_y, end_x, end_y, width, height, max_freq_value, step_width, mid_y;
uint8_t num_freq;
uint16_t last_freq_values[MAX_NUM_FREQ_VALUES];
uint8_t first_time;


void graph_clear_all() {
	int i;
	LCD_setColors(Black, Black);
	LCD_fillRect(start_x, start_y, width, height);
	for(i = 0; i < MAX_NUM_FREQ_VALUES; i++) {
		last_freq_values[i] = 0;
	}
}

void run_graph(drawGraphFunction f, uint16_t *freq_values) {
	int i;
	if (!first_time) {
		f(freq_values);
		for (i = 0; i < num_freq; i++) {
			last_freq_values[i] = freq_values[i];
		}
	} else {
		first_time = 0;
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

static void runGraphTask(void *params) {
	uint8_t counter; /* only for now to have a moving sine */
	const uint8_t max_count = 100; /* only for now */
	uint16_t *freqs = (uint16_t *) params;
	for (;;) {
		run_graph(draw_simple_rainbow_graph, &freqs[counter]);
		counter = (counter+1)%max_count;
		vTaskDelay(30 / portTICK_RATE_MS);
	}
}


void setup_graph(
	uint16_t _start_x, 
	uint16_t _start_y, 
	uint16_t _end_x, 
	uint16_t _end_y, 
	uint8_t _num_freq, 
	uint16_t _max_freq_value,
	uint16_t *_freq_array
) {
	
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
	graph_clear_all();
	xTaskCreate(runGraphTask, "graph", 100, _freq_array, 1, NULL);
}


void setup_default_graph() {
	uint8_t i;
	uint16_t _sine[200] = {
		127,134,142,150,158,166,173,181,188,195,201,207,213,219,224,229,234,238,241,245,
		247,250,251,252,253,254,253,252,251,250,247,245,241,238,234,229,224,219,213,207,
		201,195,188,181,173,166,158,150,142,134,127,119,111,103,95,87,80,72,65,58,52,46,
		40,34,29,24,19,15,12,8,6,3,2,1,0,0,0,1,2,3,6,8,12,15,19,24,29,34,40,46,52,58,65,
		72,80,87,95,103,111,119,127,134,142,150,158,166,173,181,188,195,201,207,213,219,
		224,229,234,238,241,245,247,250,251,252,253,254,253,252,251,250,247,245,241,238,
		234,229,224,219,213,207,201,195,188,181,173,166,158,150,142,134,127,119,111,103,
		95,87,80,72,65,58,52,46,40,34,29,24,19,15,12,8,6,3,2,1,0,0,0,1,2,3,6,8,12,15,19,
		24,29,34,40,46,52,58,65,72,80,87,95,103,111,119
	};
	for (i = 0; i < 200; i++) {
		sine[i] = _sine[i];
	} 
	
	setup_graph(0, 0, WIDTH, HEIGHT, 20, 256, sine);
}

