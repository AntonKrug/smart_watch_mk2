#ifndef SMARTWATCH_NEOPIXEL_H
#define SMARTWATCH_NEOPIXEL_H

#include <stdint.h> 

#define NEOPIXEL_CLOCK_COLOR       0x8F0  // Red with a hint of a purple
#define NEOPIXEL_SET_HOURS_COLOR   0x00F  // Green
#define NEOPIXEL_SET_MINUTES_COLOR 0xF00  // Blue
#define NEOPIXEL_BLACK_COLOR       0x000  // Turned off (black color is 100% off)

#define NEOPIXEL_START_FADE        16     // do a 16 step fade from 16 to 1 (0 is special state where this fade is completely disabled)


extern volatile bit     neopixelUpdate;
extern          uint8_t neopixelFadeCountDown;  


extern void neopixelFadeHandler(void);    // Fading only the red color only
extern void neopixelSetColor(uint16_t color);

#endif
