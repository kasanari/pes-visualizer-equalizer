#include "graph.h"
#include "fft.h"
#include "semphr.h"
#include "global.h"


#define MAX_NUM_FREQ_VALUES FFT_LENGTH/2

#define BAR_SPACING 3 

#define LIMIT_UPPER_VALUE(val, lim) (val < lim ? val : lim)

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
uint16_t last_freq_values[MAX_NUM_FREQ_VALUES];
uint8_t first_time;

uint16_t frequencies_to_plot[FFT_LENGTH/2];
graph_setting_t graph_info;
uint8_t last_graph_index;
drawGraphFunction current_draw_func;
uint16_t maximum = 1000;

void graph_clear_all(graph_setting_t *graph) {
	int i;
	LCD_setColors(Black, Black);
	LCD_fillRect(graph->start_x, graph->start_y, graph->width, graph->height);
	for(i = 0; i < MAX_NUM_FREQ_VALUES; i++) {
		last_freq_values[i] = 0;
	}
}

void setDrawGraphFunction(drawGraphFunction func) {
	if (current_draw_func != func) {
		current_draw_func = func;
	}
}

void run_graph(drawGraphFunction f, graph_setting_t *graph) {
	int i;
	if (!first_time) {
		f(graph->freqs, graph);
		for (i = 0; i < graph->num_freq; i++) {
			last_freq_values[i] = LIMIT_UPPER_VALUE(graph->freqs[i], graph->max_freq_value);
		}
	} else {
		first_time = 0;
	}
}

void draw_simple_white_graph(uint16_t *freq_values, graph_setting_t *graph) {
	uint16_t i;
	int16_t bar_diff, last_bar, bar;
	for (i = 0; i < graph->num_freq; i++) {
		last_bar 	= (graph->height*last_freq_values[i])/	graph->max_freq_value;
		bar	= (graph->height * LIMIT_UPPER_VALUE(freq_values[i], graph->max_freq_value)) /	graph->max_freq_value;
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			LCD_setColors(White, White);		
			LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - bar, graph->step_width-BAR_SPACING, bar_diff);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - last_bar, graph->step_width-BAR_SPACING, -bar_diff);
		}
	}
}



void draw_simple_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph) {
	uint16_t i;
	unsigned short color;
	short hue;
	int16_t bar_diff, last_bar, bar;
	for (i = 0; i < graph->num_freq; i++) {
		hue = (360*i)/graph->num_freq;
		color = getColorFromHSL(hue, 100, 50);
		LCD_setColors(color, color);
		last_bar 	= (graph->height*last_freq_values[i]) /	graph->max_freq_value;
		bar	= (graph->height * LIMIT_UPPER_VALUE(freq_values[i], graph->max_freq_value)) /	graph->max_freq_value;
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - bar, graph->step_width-BAR_SPACING, bar_diff);
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - last_bar, graph->step_width-BAR_SPACING, -bar_diff);
		}
	}
}

void draw_block_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph) {
	uint16_t i;
	unsigned short color;
	int16_t bar_diff, last_bar, bar;
	uint8_t block_height = 5;
	uint8_t block_diff = block_height - 2;
	for (i = 0; i < graph->num_freq; i++) {
		last_bar 	= (graph->height*last_freq_values[i]) /	(block_height * graph->max_freq_value);
		bar	= (graph->height * LIMIT_UPPER_VALUE(freq_values[i], graph->max_freq_value)) /	(block_height*graph->max_freq_value);
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) {
			for(int j = last_bar; j < bar; j++) {
				unsigned short hue = 300-(180*j*block_height)/graph->max_freq_value;
				color = getColorFromHSL(hue, 100, 50);
				LCD_setColors(color, color);
				LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - (j+1)*block_height, graph->step_width-BAR_SPACING, block_diff);
			}
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(graph->start_x + i*graph->step_width, graph->end_y - last_bar*block_height, graph->step_width-BAR_SPACING, -bar_diff*(block_height));
		}
	}
}

void draw_block_mirror_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph) {
	uint16_t i;
	unsigned short color;
	int16_t bar_diff, last_bar, bar, mirror_lightness;
	uint8_t block_height = 10;
	uint8_t block_diff = block_height - 7;
	short hue;
	
	for (i = 0; i < graph->num_freq; i++) {
		last_bar 	= (graph->height*last_freq_values[i])/	(block_height * graph->max_freq_value);
		bar	= (graph->height * LIMIT_UPPER_VALUE(freq_values[i], graph->max_freq_value)) / (block_height*graph->max_freq_value);
		bar_diff 	= bar - last_bar;
		if (bar_diff > 0) { 
			for (int j = last_bar; j < bar; j++) {
				hue = 240-15*j;
				color = getColorFromHSL(hue, 100, 50);
				LCD_setColors(color, color);
				LCD_fillRect(graph->start_x + i*graph->step_width, graph->mid_y - j*block_height/2, graph->step_width-BAR_SPACING, block_diff);
				
				// Mirrored
				mirror_lightness = 45-4*j;
				if (mirror_lightness > 0) {
					color = getColorFromHSL(hue, 50+mirror_lightness, mirror_lightness );
					LCD_setColors(color, color);
					LCD_fillRect(graph->start_x + i*graph->step_width, graph->mid_y + (j+1)*block_height/2, graph->step_width-BAR_SPACING, block_diff);	
				}
			}
		} else if(bar_diff < 0) {
			LCD_setColors(Black, Black);
			LCD_fillRect(graph->start_x + i*graph->step_width, 	graph->mid_y - (last_bar-1)*block_height/2, graph->step_width-BAR_SPACING, 	-bar_diff*(block_height)/2-1);
			LCD_fillRect(graph->start_x + i*graph->step_width, 	graph->mid_y + (bar+1)*block_height/2, 			graph->step_width-BAR_SPACING, 	-bar_diff*(block_height)/2-1);
		}
	}
}

graph_setting_t *setup_graph(
	uint16_t _start_x, 
	uint16_t _start_y, 
	uint16_t _end_x, 
	uint16_t _end_y, 
	uint8_t _num_freq, 
	uint16_t _max_freq_value,
	uint16_t *_freq_array
) {
	
	graph_info.start_x = _start_x; 
	graph_info.start_y = _start_y;
	graph_info.end_x = _end_x;  
	graph_info.end_y = _end_y;  
	graph_info.num_freq = _num_freq;
	graph_info.max_freq_value = _max_freq_value;
	
	graph_info.width = _end_x - _start_x;
	graph_info.height = _end_y - _start_y;	
	graph_info.step_width = graph_info.width/_num_freq;
	graph_info.mid_y = _start_y + graph_info.height/2;
	graph_info.freqs = _freq_array;
	first_time = 1;
	
	// Set last frequency array to zero
	graph_clear_all(&graph_info);
	//xTaskCreate(runGraphTask, "graph", 100, graph, 1, NULL);
	return &graph_info;
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

void float_to_int(FLOAT_TYPE *float_buf, uint16_t*int_buf, int n, int mask) {
	int temp;
	for (int i = 0; i < n; i++) {
		temp = mask * float_buf[i];
		int_buf[i] = (temp > maximum) ? maximum : temp; 
	}
}

uint16_t max(uint16_t *buf, size_t n) {
		uint16_t max = 0;
		for (int i = 0; i < n; i++) {
			if (buf[i] > max) {
				max = buf[i];
			}
		}
		return max;
}

uint16_t min(uint16_t *buf, size_t n) {
		uint16_t min = 0;
		for (int i = 0; i < n; i++) {
			if (buf[i] < min) {
				min = buf[i];
			}
		}
		return min;
}

static void FFTGraphTask(void *params) {
	context_t *ctx = (context_t*)params;
	graph_setting_t *graph = setup_graph(0, 60, WIDTH, HEIGHT, FFT_LENGTH/2, maximum, frequencies_to_plot);
	uint8_t current_graph_index;
	for (;;) {
		current_graph_index = *ctx->graph_index;
		xSemaphoreTake(ctx->lcd_lock, portMAX_DELAY);
		if (last_graph_index != current_graph_index) {
			graph_clear_all(graph);
			last_graph_index = current_graph_index;
		}
		run_graph(ctx->graphs[current_graph_index].graph_func, graph);
		xSemaphoreGive(ctx->lcd_lock);
		xSemaphoreGive(ctx->signals->graph_done_lock);
		xSemaphoreTake(ctx->signals->fft_done_lock, portMAX_DELAY);
		float_to_int(ctx->signals->magnitude, frequencies_to_plot, FFT_LENGTH/2, 1);
		vTaskDelay(30 / portTICK_RATE_MS);
	}
}

void plot_fft_graph(context_t *ctx) {
	last_graph_index = NUM_OF_GRAPHS;
	xTaskCreate(FFTGraphTask, "graph", 1000, ctx, 1, NULL);
}

