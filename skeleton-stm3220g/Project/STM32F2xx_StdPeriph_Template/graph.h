#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdint.h>
#include <stdlib.h>

#include "color.h"
#include "LCD.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fft.h"

#define NUM_OF_GRAPHS 4


typedef struct graph_setting {
	uint16_t start_x, start_y, end_x, end_y, width, height, max_freq_value, step_width, mid_y;
	uint8_t num_freq;
	uint16_t *freqs;
} graph_setting_t;

typedef void (*drawGraphFunction)(uint16_t*, graph_setting_t*);

typedef struct graph_type {
	drawGraphFunction graph_func;
	char* name;
} graph_type_t;


typedef struct context {
	xSemaphoreHandle lcd_lock;
	graph_type_t *graphs;
	uint8_t *graph_index;
	FFT_signals_t *signals;
} context_t;



/*
	PRE: _start_x < _end_x, _start_y < end_y, 0 < _num_freq < 32
*/
graph_setting_t *setup_graph(
	uint16_t _start_x,
	uint16_t _start_y, 
	uint16_t _end_x, 
	uint16_t _end_y, 
	uint8_t _num_freq, 
	uint16_t _max_freq_value,
	uint16_t *_freq_array
);

void setup_default_graph(void);

void plot_fft_graph(context_t *ctx);

void graph_clear_all(graph_setting_t *graph);

void run_graph(drawGraphFunction f, graph_setting_t *graph);

void draw_simple_white_graph(uint16_t *freq_values, graph_setting_t *graph);

void draw_simple_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph);

void draw_block_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph);

void draw_block_mirror_rainbow_graph(uint16_t *freq_values, graph_setting_t *graph);


#endif
