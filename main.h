#ifndef SMARTWATCH_MAIN_H
#define SMARTWATCH_MAIN_H

#include <stdint.h> // `uint8_t` and `uint16_t` 

#define SYSTICK_MAX       20 // Systick will count only from 0 to 19
#define PRESS_TO_SET_TIME 40 // 40 * 0.05s  = 2.0s press to trigger the 'set time'
#define PRESS_SHORT       10 // 10 * 0.05s  = 0.5s for short press


extern volatile uint8_t systick; // 20Hz systick counter

#endif
