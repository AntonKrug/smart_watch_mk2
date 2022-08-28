#ifndef SMARTWATCH_NEOPIXEL_H
#define SMARTWATCH_NEOPIXEL_H

#include <stdint.h> 

#define NEOPIXEL_CLOCK_COLOR       0x8F0  // Red with a hint of a purple
#define NEOPIXEL_SET_HOURS_COLOR   0x00F  // Green
#define NEOPIXEL_SET_MINUTES_COLOR 0xF00  // Blue
#define NEOPIXEL_BLACK_COLOR       0x000  // Turned off

extern void setNeopixel(uint16_t color);

#endif
