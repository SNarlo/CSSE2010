/*
 * level.h
 *
 * Created: 31/05/2020 6:02:29 PM
 *  Author: s4434571
 */ 

#include <stdint.h>

uint8_t get_level(void);
void init_level(void);
void increase_level();
void check_if_level_up(void);
void level_up_spash_screen(void);