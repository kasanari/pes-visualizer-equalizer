#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdint.h>
#include <stdlib.h>

#include "color.h"
#include "LCD.h"
#include "FreeRTOS.h"
#include "task.h"

typedef void (*drawGraphFunction)(uint16_t*);

/*
	PRE: _start_x < _end_x, _start_y < end_y, 0 < _num_freq < 32
*/
void setup_graph(
	uint16_t _start_x,
	uint16_t _start_y, 
	uint16_t _end_x, 
	uint16_t _end_y, 
	uint8_t _num_freq, 
	uint16_t _max_freq_value,
	uint16_t *_freq_array
);

void setup_default_graph(void);

void graph_clear_all(void);

void run_graph(drawGraphFunction f, uint16_t *freq_values);

void draw_simple_white_graph(uint16_t *freq_values);

void draw_simple_rainbow_graph(uint16_t *freq_values);

void draw_block_rainbow_graph(uint16_t *freq_values);

void draw_block_mirror_rainbow_graph(uint16_t *freq_values);


#endif
