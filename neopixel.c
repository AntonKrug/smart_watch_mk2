#include <stdint.h>

#include "neopixel.h"
#include "vfd.h"


uint8_t      neopixelFadeCountDown = 0;  // Count down for how the Neopixel increase in brightness should be done  
volatile bit neopixelUpdate        = 0;  // A 'update flag' set true at the systick frequency
    

// Swap the order of the lower 4 bits between each other (the higher 4-bits will be discarded)
uint8_t reverse4bits(uint8_t value) {
  return ((value & (1<<0)) << 3) |  // Move bit 0 to bit 3
         ((value & (1<<1)) << 1) |  // Move bit 1 to bit 2          
         ((value & (1<<2)) >> 1) |  // Move bit 2 to bit 1
         ((value & (1<<3)) >> 3);   // Move bit 3 to bit 0
}                        


// Fading only the red color only
void neopixelFadeHandler(void) {
  if (neopixelUpdate) {
    // Consider doing the Neopixel update only once per systick
    
    if (neopixelFadeCountDown) {
      // At the first 16 ticks of the systick timer the color will fade from black to red
      // only if the neopixelFadeUp is setup. This way the fade will happen only on
      // powerup but not any other button presses which reset the stayAwake counter     
      
      neopixelFadeCountDown--;
      if (neopixelFadeCountDown) {
        neopixelSetColor(reverse4bits((NEOPIXEL_START_FADE - neopixelFadeCountDown)/2) << 4);
      } else {
        neopixelSetColor(NEOPIXEL_CLOCK_COLOR); // Final color is reached
      }      
    }
    
    if (16 > stayAwake) {
      // At the last 16 ticks of the systick timer the color will fade from red to black
      neopixelSetColor(reverse4bits((stayAwake)/2) << 4);
    }
          
    neopixelUpdate = 0;
  }
}


// Set color of one Neopixel WS2812B LED.
// Hardcoded to PB2 pin and timed exactly for an 8MHz clock.
// 12-bit colors instead of 24-bit, (4-bit for each channel, each bit is transmitted twice). 
// Order of colors: Blue Red Green (0xBRG)
// MSB is first in the stream, this means that 0x001 will be a brighter green than 0x008
// CodeVisionAVR compiler doesn't support good mixing of C and ASM and
// had to hardcode registers for its ABI. While rewriting  this into GCC
// and its "Extended assembly" would make it more portable and robust.
void neopixelSetColor(uint16_t color) {
  #asm 
    cli              // Disable IRQ
    ldi  r31, 12     // counter=12  (12 bits to count down), the ABI promising R31 is free to use in assembly
    lsr  r17         // color = color >> 1    hardcoding R16 and R17 for CodeVisionAVR ABI :( 
    ror  r16    
    brcc _bit_low    // If the bit shifted from color is 0 then push the low bits, otherwise push the high bits 
       
  _bit_high:
    sbi  0x5, 2      // PB2 = 1
    nop              // Timing NOPs
    nop
    nop  
    lsr  r17         // Shift the color bits again, sooner than it's needed (because later there will be no time) 
    ror  r16
    dec  r31         // counter--; it doesn't touch the CARRY, so both decrement and right shift are safe to do ahead of time    
    cbi  0x5, 2      // PB2 = 0
    nop           
    nop 
    sbi  0x5, 2      // PB2 = 1 (each bit is pushed twice)   
    nop
    nop
    nop
    breq _neo_end    // If (0==counter) then end this loop (the end of the loop will do CBI anyway, so it's safe to do it ahead of time)
    nop 
    cbi  0x5, 2      // PB2 = 0 (but instead of timing NOPs do some decisions)
    brcc _bit_low    // If the carry from the shifted bit from the color was 0 then go to the low bit branch,
    rjmp _bit_high   // otherwise repeat this high bit branch
                     
  _bit_low:
    sbi  0x5, 2      // PB2 = 1
    nop              // Timing NOP
    cbi  0x5, 2      // PB2 = 0
    nop 
    nop 
    lsr  r17         // Shift the color bits again, sooner than it's needed (because later there will be no time) 
    ror  r16
    dec  r31         // counter--; it doesn't touch the CARRY, so both decrement and right shift are safe to do ahead of time
    sbi  0x5, 2      // PB2 = 1 (each bit is pushed twice)   
    nop              // Timing NOP
    cbi  0x5, 2      // PB2 = 0
    nop
    nop 
    breq _neo_end    // If (0==counter) then end this loop
    brcc _bit_low    // If the carry from the shifted bit from the color was 0 then repeat this low bit branch,
    rjmp _bit_high   // otherwise go to the high bit branch
      
  _neo_end:      
    cbi  0x5,2       // PB2 = 0 as it might not have been set yet
                     // Leave it at 0 as that will send the WS2812B chip a 'reset' command which will start displaying the color
                    
    sei              // enable IRQs
  #endasm
}


