#ifndef SMARTWATCH_VFD_H
#define SMARTWATCH_VFD_H

#include <stdint.h>     // `uint8_t` and `uint16_t` 


#define SLEEP_TIMEOUT 22    // 22 * 0.5s = 11s timeout before turning off the VFD
#define PRESS_TO_SET_TIME 4 // 4 * 0.5s  = 2.0s presses


// Character selectors
#define VFD_CH_1 6      // Hours major
#define VFD_CH_2 5      // Hours minor
#define VFD_CH_3 3      // The : character between the hours and minutes -> HH:MM
#define VFD_CH_4 1      // Minutes major
#define VFD_CH_5 11     // Minutes minor

// Individual segments of a 7-segment character 
// https://en.wikipedia.org/wiki/Seven-segment_display
//  --A--
// |     |
// F     B
// |     |
//  --G--
// |     |
// E     C
// |     |
//  --D--

#define VFD_A 10
#define VFD_B 0
#define VFD_C 4
#define VFD_D 2
#define VFD_E 8
#define VFD_F 9
#define VFD_G 7


extern volatile uint8_t displayDots; // Twice a second flip between displaying the dots and displaying nothing
extern volatile uint8_t stayAwake;   // How long before going to sleep (2Hz counter counting to 0)


void vfdOn(void);                    // Turn on both DC2DC and filament heater
void vfdOff(void);                   // Turn off both DC2DC and filament heater

void sendDataToVfd(uint16_t data);              // Transfer 1 character to VFD

void displayTime(uint8_t hour, uint8_t minute); // Display HH:MM on the VFD


#endif
