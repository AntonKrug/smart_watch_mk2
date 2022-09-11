#ifndef SMARTWATCH_NEOPIXEL_H
#define SMARTWATCH_NEOPIXEL_H

#include <stdint.h> 
                                             
#define NEOPIXEL_CLOCK_COLOR       0x0C0       // Red
#define NEOPIXEL_SET_HOURS_COLOR   0x002       // Green
#define NEOPIXEL_SET_MINUTES_COLOR 0x200       // Blue
#define NEOPIXEL_BLACK_COLOR       0x000       // Turned off (black color is 100% off)

#define NEOPIXEL_START_FADE        4           // do a 4-step fade from 4 to 1 (0 is a special state where this fade is disabled)

#define NEOPIXEL_FADE_SPEED        4           // clock divisor how quickly the fade increments/decrements will happen


extern volatile bit     neopixelUpdate;        // Update the Neopixel just once per systick
extern          uint8_t neopixelFadeCountDown; // Counter starting at NEOPIXEL_START_FADE counting to 0 to track the fade


extern void neopixelFadeHandler(void);         // Fading only the red color only
extern void neopixelSetColor(uint16_t color);  // Send a 12-bit color to a single Neopixel

#endif
