#ifndef SMARTWATCH_MAIN_H
#define SMARTWATCH_MAIN_H

#include <stdint.h> // `uint8_t` and `uint16_t` 

#define SYSTICK_MAX 10 // Systick will count only from 0 to 9

extern volatile uint8_t systick; // 10Hz systick counter

#endif
