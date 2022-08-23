#include <mega88a.h> // AVR Mega88 PA
#include <delay.h>   // Delay for-loop functions
#include <spi.h>     // SPI functions to talk with MAX6920SAWP -> VFD
#include <stdint.h>  // `uint8_t` and `uint16_t` 

#include "vfd.h"


volatile uint8_t displayDots = 0;             // Twice a second flip between displaying the dots and displaying nothing
volatile uint8_t stayAwake   = SLEEP_TIMEOUT; // How long before going to sleep (2Hz counter counting to 0)


// Turn off VFD's high-voltage DC2DC boost converter (PD1)
void dc2dcOff(void) {
  PORTD = PORTD & (~(1 << PORTD1));
}


// Turn on VFD's high-voltage DC2DC boost converter (PD1) 
void dc2dcOn(void) {
  PORTD = PORTD | (1 << PORTD1);
}


// Turn off VFD's low-voltage filament heater (PB1) 
void fHeatOff(void) {
  PORTB = PORTB & (~(1 << PORTB1));
}


// Turn on VFD's low-voltage filament heater (PB1) 
void fHeatOn(void) {
  PORTB = PORTB | (1 << PORTB1);
}


// Turn off both DC2DC and filament heater
void vfdOff() {
  dc2dcOff();
  fHeatOff();
}


// Turn on both DC2DC and filament heater
void vfdOn() {
  dc2dcOn();
  fHeatOn();
  delay_us(500); // Give time for DC2DC to stabilise before displaying the time 
}


// Set high or low the PD7 pin -> MAX6920AWP.LOAD signal.
// Allowing the serially  shifted data to be read into the driver stage
void setVfdLoad(uint8_t value) {
  PORTD = (PORTD & (~(1 << PORTD7))) | (value << PORTD7);
}


// Transfer data to VFD, at the time only 1 character will be displayed,
// to display full clock this call needs to be called 4-5 times
void sendDataToVfd(uint16_t data) {
  // Transfer 16-bits to the MAX6920AWP, only lower 12-bits will be kept, 
  // bit 12-15 will be truncated in the MAX6920AWP chip
  spi(data >> 8);
  spi(data & 0xff);
               
  // Start displaying the data on VFD 
  setVfdLoad(1);
  delay_us(2);
  
  setVfdLoad(0);
  delay_us(10);
}


// Take `hour` and `minute` values and send the corresponding data
// to VFD which will display it
void displayTime(uint8_t hour, uint8_t minute) {

  //  --A--
  // |     |
  // F     B
  // |     |
  //  --G--
  // |     |
  // E     C
  // |     |
  //  --D--

  const uint16_t segments[] = {
    1 << VFD_A | 1 << VFD_B | 1 << VFD_C | 1 << VFD_D | 1 << VFD_E | 1 << VFD_F,               // 0 character
    1 << VFD_B | 1 << VFD_C,                                                                   // 1 character
    1 << VFD_A | 1 << VFD_B | 1 << VFD_G | 1 << VFD_E | 1 << VFD_D,                            // 2 character
    1 << VFD_A | 1 << VFD_B | 1 << VFD_G | 1 << VFD_C | 1 << VFD_D,                            // 3 character
    1 << VFD_F | 1 << VFD_G | 1 << VFD_B | 1 << VFD_C,                                         // 4 character
    1 << VFD_A | 1 << VFD_F | 1 << VFD_G | 1 << VFD_C | 1 << VFD_D,                            // 5 character
    1 << VFD_F | 1 << VFD_G | 1 << VFD_C | 1 << VFD_D | 1 << VFD_E,                            // 6 character
    1 << VFD_A | 1 << VFD_B | 1 << VFD_C,                                                      // 7 character
    1 << VFD_A | 1 << VFD_B | 1 << VFD_C | 1 << VFD_D | 1 << VFD_E | 1 << VFD_F | 1 << VFD_G,  // 8 character
    1 << VFD_A | 1 << VFD_F | 1 << VFD_B | 1 << VFD_G | 1 << VFD_C,                            // 9 character
  };                                     
  
  // Hours                                        
  uint8_t hourTens = hour / 10;
  sendDataToVfd((0 == hourTens) ? 0 : segments[hourTens] | 1 << VFD_CH_1); // display blank if it's leading 0    
  sendDataToVfd(segments[hour % 10]                      | 1 << VFD_CH_2);             
                    
  // The ':' dots
  sendDataToVfd(0                                        | displayDots << VFD_CH_3);
         
  // Minutes
  sendDataToVfd(segments[(minute / 10) % 60]             | 1 << VFD_CH_4);    
  sendDataToVfd(segments[minute % 10]                    | 1 << VFD_CH_5);    
                      
  // Make sure that characters are displayed for equal time and have equal brightness,  
  // therefore leave the VFD in an off state                                                       
  sendDataToVfd(0);   
}
