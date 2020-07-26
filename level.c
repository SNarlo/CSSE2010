/*
 * level.c
 *
 * Created: 31/05/2020 6:00:28 PM
 *  Author: Sebastian Narloch 
 */ 



#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "player.h"
#include "alien.h"
#include "game_background.h"
#include "projectile.h"
#include "level.h"
#include "game_background.h"

#define F_CPU 8000000L
#include <util/delay.h>


volatile uint8_t level = 1;
uint16_t count = 1;



uint8_t get_level(void) {
	return level;
}

void init_level(void) {
	move_cursor(6, 6);
	printf_P(PSTR("Level"));
	move_cursor(0,7);
	printf_P(PSTR("% 10d"), get_level());
}

void increase_level(void) {
	level ++;	
	increment_level_counter();
}

void check_if_level_up(void) {
	uint32_t extra = get_score() % 100; // get the values over 100
	if (get_score() > 100 && ((get_score() - extra) % 100 == 0)) { // check if score is divisible by 100
		if (count == level) { 
			increase_level(); // 
			level_up_spash_screen();		
		}
	}
}

void level_up_spash_screen(void) {
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("LEVEL UP", COLOUR_GREEN);
		// Scroll the message until it has scrolled off the
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != NO_BUTTON_PUSHED || serial_input_available()) {
				clear_serial_input_buffer();
				init_background();
				init_player();
				return;
			}
		}
	}
}