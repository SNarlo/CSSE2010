/*
 * score.h
 * 
 * Author: Peter Sutton
 */

#ifndef SCORE_H_
#define SCORE_H_

#include <stdint.h>

void init_score(void);
void add_to_score(uint16_t value);
uint32_t get_score(void);
void update_serial(void);
void update_high_score(void);
uint32_t get_high_score(void);
void check_score(void);
void display_digit(uint8_t number, uint8_t digit);
void add_kill_shot(uint16_t value);
void seven_seg_score(void);
#endif /* SCORE_H_ */