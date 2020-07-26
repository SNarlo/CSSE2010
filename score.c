/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include "terminalio.h"
#include "level.h"


#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>

uint32_t score;
uint32_t high_score;


void init_score(void) {
	score = 0;
	move_cursor(6, 2); // go to score line
	printf_P(PSTR("Score")); //print score
	move_cursor(0, 3); // move to the next line
	printf_P(PSTR("% 10d"), score); // print the initial score
	
	move_cursor(1, 4); // move to the next line
	printf_P(PSTR("High Score")); // print high score
	move_cursor(0, 5); // move to next line
	printf_P(PSTR("% 10d"), get_high_score()); // print high score value
}

void add_to_score(uint16_t value) {
	score += value;
}

void add_kill_shot(uint16_t value) {
	score += value;
}

uint32_t get_score(void) {
	return score;
}

void update_high_score(void) {
	if (score <= high_score) {
		;
		} else {
		high_score = score;
	}
}

uint32_t get_high_score(void) {
	return high_score;
}


void update_serial(void) {

	move_cursor(6, 2); // go to score line
	printf_P(PSTR("Score")); //print score
	move_cursor(0, 3); // move to next line right aligned
	printf_P(PSTR("% 10d"), get_score()); // print the score after 10 spaces
	update_high_score(); // check if the score is the high score
	
	move_cursor(1, 4); // go to high score line
	printf_P(PSTR("High Score")); // print high score
	move_cursor(0, 5); // move to next line
	printf_P(PSTR("% 10d"), get_high_score()); // print high score value
	
	check_if_level_up();
	
	//check for level as well 
	move_cursor(6, 6);
	printf_P(PSTR("Level"));
	move_cursor(0,7);
	printf_P(PSTR("% 10d"), get_level());
}



