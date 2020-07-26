/*
 * timer0.c
 *
 * Author: Peter Sutton
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "score.h" // import score to use timer counter on score
#include <math.h>
#include "timer0.h"
#include "player.h"


/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clockTicks;

// Double speed mode (Odd = Off, Even = On)
uint32_t double_speed = 1;

/* Set up timer 0 to generate an interrupt every 1ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer0(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clockTicks = 0L;
	
	/* Clear the timer */
	TCNT0 = 0;

	/* Set the output compare value to be 124 */
	OCR0A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01)|(1<<CS00);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK0 |= (1<<OCIE0A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR0 |= (1<<OCF0A);
}

uint32_t get_current_time(void) {
	uint32_t returnValue;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	returnValue = clockTicks;
	if(interruptsOn) {
		sei();
	}
	return returnValue;
}

void increment_double_speed(void) {
	double_speed++;
}

uint32_t get_double_speed(void) {
	return double_speed;
}

void reset_double_speed(void) {
	if (double_speed % 2 == 0){ // reset DP if on
		PORTD ^= (1 << 6);	
	}
	
	double_speed = 1; // rest the counter
}


uint8_t seven_seg[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111}; // 0-9
volatile uint8_t seven_seg_cc = 0; // left  = 1 right = 0;

ISR(TIMER0_COMPA_vect) {
	/* Increment our clock tick count */
	clockTicks++;
	
	// switch between left and right display
	seven_seg_cc = 1 ^ seven_seg_cc;	
	
	// double speed DP	
	if (double_speed % 2 == 0 && seven_seg_cc == 0) { // if the right display is on and double speed is on
		PORTD |= (1 << 6); // turn DP on
	}
	
	//get the score
	int32_t game_score = (int32_t) get_score();
	
	if (game_score > 99) {
		// make score stay at 99 if over
		game_score = 99;
		
	}
	// output score to Seven seg display
	if (seven_seg_cc == 0) {
		PORTC = seven_seg[game_score%10] | (0 << 7); // set one value
	} else if (game_score > 9) {
		uint8_t digit = floor(game_score/10); // get the 10 value
		PORTC = seven_seg[digit] | (1 << 7); // set left seven seg to 10 value once score is over 9	
	} 
}


	
	