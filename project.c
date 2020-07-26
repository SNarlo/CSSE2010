/*
 * project.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by Sebastian Narloch
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

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

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void seven_seg_ports(void);
void init_health_bar(void);
uint8_t get_lives(void);
void show_lives(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

//Pause state for game (0 = not paused, 1 = paused)
uint8_t paused = 0;
// Four lives
uint8_t lives = 4;
//joystick axis
uint8_t x_y_axis = 0;
//joystick value
uint16_t value = 0;

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware. This will turn on interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
		update_serial();
		show_lives();
	}
}

// for a given frequency (Hz), return the clock period
uint16_t freq_to_clock_period(uint16_t frequency) {
	return (1000000UL/frequency);
}

uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clock_period) {
	return (dutycycle * clock_period) / 100;
}

//void play_sound(uint32_t frequency) {
	//DDRA |= (1<<2); // set to output
	//PORTA |= (1 << 2); // turn on buzzer
	//
	//float duty_cycle = 50; 
	//uint16_t clockperiod = freq_to_clock_period(frequency);
	//uint16_t pulsewidth = duty_cycle_to_pulse_width(duty_cycle, clockperiod);
	//
	//// Set the maximum count value for time counter 1 to be one less than the clock period
	//OCR1A = clockperiod - 1;
	//
	//// Set the count compare value based on the pulse width. The value will be 1 less 
	//// than the pulse width, unless the pulsewidth is 0
	//
	//if (pulsewidth == 0) {
		//OCR1B = 0;
		//
	//} else {
		//OCR1B = pulsewidth - 1;
	//}
	//
	//// set up timer counter for fast PWM
	//TCCR1A = (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11)| (1 << WGM10);
	//TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12)| (1 << CS11) | (1 << CS10);
	//
	//PORTA ^= (1 << 2); // turn off the buzzer
//
//}

// sound for when a projectile is fired
void projectile_sound(void) {	
	//play_sound(5000);
} 

// sound for when player dies
void death_sound(void) {
	//play_sound(20000);
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	
	// Setup serial port for 38400 baud communication with no echo
	// of incoming characters
	init_serial_stdio(38400,0);
	
	init_timer0();
	
	init_health_bar();
	
	// Turn on global interrupts
	sei();
	
	seven_seg_ports(); // initialise seven seg dispay
	
	
	// set up ADC
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	
	
}

void seven_seg_ports(void) {
	DDRC = 0xFF; // Set all PORTC pins to be outputs
}


void init_health_bar(void) {
	DDRD |= 0xFF; // set Port D to output
	
	PORTD |= (1<<2) | (1<<3) | (1<<4) | (1<<5); // turn on LED's
}

// method for handling LED health bar and death sound
void handle_death(void) {
	if (lives == 4) {
		PORTD ^= (1<<2); // turn off the LED
		_delay_ms(2000); // delay for 2 seconds after death
	} else if (lives == 3) {
		PORTD ^= (1<<5);
		_delay_ms(2000);
	} else if (lives == 2) {
		PORTD ^= (1<<3);
		_delay_ms(2000);
	} else if (lives == 1) {
		PORTD ^= (1 << 4);
	}
}

uint8_t get_lives() {
	return lives;
}

// prints the amount of lives to the terminal 
void show_lives(void) {
	move_cursor(6,8);
	printf_P(PSTR("Lives"));
	move_cursor(0,9);
	printf_P(PSTR("% 10d"), get_lives());
}

// method for joystick functionality
void joystick_functionality(void) {
	// joystick functionality
	if (x_y_axis == 0) {
		ADMUX &= ~1;
		} else {
		ADMUX |= 1;
	}
	
	// ADC conversion
	ADCSRA |= (1 << ADSC);
	
	while (ADCSRA & (1<<ADSC)) {
		;
	}
	
	value = ADC; // read the value
	
	if (x_y_axis == 0) { // x axis	
		if (value > 700) {
			move_player_left();
		} else if (value < 300) {
			move_player_right();
		}
	} else if (x_y_axis == 1) { // y axis
			if (value > 700 ) {
				move_player_up();
			} else if (value < 300) {
				move_player_down();
		}
		
	}
	x_y_axis ^= 1;
}


void splash_screen(void) {
	// Reset display attributes and clear terminal screen then output a message
	set_display_attribute(TERM_RESET);
	clear_terminal();
	
	hide_cursor();	// We don't need to see the cursor when we're just doing output
	move_cursor(3,3);
	printf_P(PSTR("Space Impact"));
	
	move_cursor(3,5);
	set_display_attribute(FG_GREEN);	// Make the text green
	printf_P(PSTR("CSSE2010/7201 project by Sebastian Narloch (44345714)"));	
	set_display_attribute(FG_WHITE);	// Return to default colour (White)
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("SPACE IMPACT  SEBASTIAN NARLOCH 44345714", COLOUR_ORANGE);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != NO_BUTTON_PUSHED || serial_input_available()) {
				clear_serial_input_buffer();
				return;
			}
		}
	}
} 
  

void new_game(void) {
	// Initialise the game elements and display
	
	init_background();	
	init_aliens();
	init_projectiles();
	init_player();

	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the score
	if (lives == 4) {
		init_score();
	}
	update_serial();
	init_level();
	show_lives();
	
		
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
	
	// Delay for half a second
	_delay_ms(500);
}

void play_game(void) {
	uint32_t current_time, last_move_time, last_alien_add_time, last_alien_move_time;
	uint32_t last_projectile_move_time;
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;

	
	
	// Get the current time and remember this as the last time any of our events
	// happened.
	current_time = get_current_time();
	last_move_time = current_time;
	last_alien_add_time = current_time;
	last_alien_move_time = current_time;
	last_projectile_move_time = current_time;
	
	// We play the game while the player isn't dead
	while(!is_player_dead()) {
	
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. We will be processing each character
		// independently and can't do anything until we get the third character.
		// At most one of the following three variables will be set to a value 
		// other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		if(button == -1) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		
		if (!paused) {
			// Process the input.
			if(button==3 || serial_input == ' ') {
				// Button 3 pressed or space bar
				fire_projectile_if_possible();
				projectile_sound();
				} else if (escape_sequence_char=='D') {
				// Left cursor key escape sequence received
				move_player_left();
				} else if(escape_sequence_char=='C') {
				// Right cursor key
				move_player_right();
				} else if(button == 0 ||escape_sequence_char=='B') {
				// Button 0 or down cursor key pressed - attempt to move down
				move_player_down();
				} else if(button == 1 || escape_sequence_char=='A') {
				// Button 1 or up cursor key pressed - attempt to move up
				move_player_up();
				} else if(button == 2) {
					increment_double_speed(); // increment the speed mode
					if (get_double_speed() % 2 == 0) { // if the speed mode is even
						move_cursor(10, 13);
						printf_P(PSTR("DOUBLE SPEED MODE")); // print double speed mode
					} else if (get_double_speed() % 2 == 1) {
						move_cursor(10, 13);
						printf_P(PSTR("                   ")); // otherwise clear	
						PORTD ^= (1 << 6); // turn off decimal point
					}
				} else if (serial_input == 'n' || serial_input == 'N') {
					new_game();
				} 
		}
		
		if(serial_input == 'p' || serial_input == 'P') {
			paused = !paused;
			
			if (paused) {
				set_display_attribute(FG_GREEN);
				set_display_attribute(TERM_BRIGHT);
				move_cursor(10,16);
				printf_P(PSTR("Paused"));
				normal_display_mode();
				move_cursor(10,17);
			} else {
				move_cursor(10,16);
				printf_P(PSTR("       "));
				move_cursor(10,17);
			}
		}
		
		if (paused) {
			if (serial_input == 'n' || serial_input == 'N') {
				new_game();
			}
		}
		

		current_time = get_current_time();
		if(!is_player_dead() && !paused &&  get_double_speed() % 2 == 1) {
			if (current_time >= last_move_time + 650) {
				// 600ms (0.6 second) has passed since the last time we scrolled
				// the background, so scroll it now and check whether that means
				// we've finished the level. (If a crash occurs we will drop out of 
				// the main while loop so we don't need to check for that here.
				scroll_background(); // stopped working for some reason, had to put in move_alien
				last_move_time = current_time;
			}
			
			// joystick
			if(current_time > last_move_time + 200) {
				joystick_functionality();
				last_move_time = current_time;
			}
			if(current_time > last_alien_add_time + 1000) {
				// 1 second has passed since the last time we tried to add an 
				// alien - so try now
				add_alien_to_game();
				last_alien_add_time = current_time;
			}
			if(current_time > last_alien_move_time + 400) {
				// 400ms has passed since the last time we tried to move an 
				// alien - so try now
				move_random_alien();
				scroll_background();
				last_alien_move_time = current_time;
			}
			if(current_time > last_projectile_move_time + 300) {
				// 300ms has passed since the last projecile move - try now
				advance_projectiles();
				last_projectile_move_time = current_time;
			}
		}
		
	
		else if (!is_player_dead() && !paused && get_double_speed() % 2 == 0) {
	
			if(current_time > last_move_time + 300) {
				// 300ms (0.3 second) has passed since the last time we scrolled
				// the background, so scroll it now and check whether that means
				// we've finished the level. (If a crash occurs we will drop out of
				// the main while loop so we don't need to check for that here.)
				
				scroll_background();
				last_move_time = current_time;
			}
			
			// joystick
			if(current_time >= last_move_time + 200) {
				joystick_functionality();
				last_move_time = current_time;
			}
			
			if(current_time > last_alien_add_time + 1000) {
				// 1 second has passed since the last time we tried to add an
				// alien - so try now
				add_alien_to_game();
				last_alien_add_time = current_time;
			}
			if(current_time > last_alien_move_time + 200) {
				// 400ms has passed since the last time we tried to move an
				// alien - so try now
				move_random_alien();
				scroll_background();
				last_alien_move_time = current_time;
			}
			if(current_time > last_projectile_move_time + 150) {
				// 300ms has passed since the last projecile move - try now
				advance_projectiles();
				last_projectile_move_time = current_time;
			}
			
		}
	}
	handle_death();
	
}


void handle_game_over() {
	if (lives == 1) {
		reset_level_counter();
		move_cursor(10,14);
		// Print a message to the terminal.
		printf_P(PSTR("GAME OVER"));
		move_cursor(10,15);
		printf_P(PSTR("Press a button to start again"));
		while(button_pushed() == -1) {
			; // wait until a button has been pushed
		}
		lives = 4;
		init_health_bar();
	} else {
		lives -= 1;
	}
	
}
