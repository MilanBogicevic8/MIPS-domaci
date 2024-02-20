/*
 * keypad.h
 *
 *  Created on: Jan 9, 2024
 *      Author: Korisnik
 */

#ifndef CORE_INC_KEYPAD_H_
#define CORE_INC_KEYPAD_H_


#include <stdint.h>

void Key_init();

extern char volatile keys[2];
extern uint32_t volatile temp_granica;


#endif /* CORE_INC_KEYPAD_H_ */
