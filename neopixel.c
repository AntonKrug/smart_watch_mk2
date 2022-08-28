#include <stdint.h>

#include "neopixel.h"


// 12-bit colors instead of 24-bit, (4-bit for each channel) 
// Order colors: Blue Red Green (0xBRG)
// MSB is first means that 0x1 will be a brighter green than 0x8
// Hardcoded to PB2 pin and timed exactly for a 8MHz clock
void setNeopixel(uint16_t color) {
  #asm 
    cli              // disable IRQ
    ldi  r31, 12     // counter=12  (12 bits to count down)
    lsr  r17         // color = color >> 1
    ror  r16    
    brcc _bit_low    // if the bit shifted from color is 0 then push the low bits, otherwise push the high bits 
       
  _bit_high:
    sbi  0x5, 2      // PB2 = 1
    nop              // Timing NOPs
    nop
    nop  
    lsr  r17         // shift the color bits again, sooner than it's needed (because later there will be no time) 
    ror  r16
    dec  r31         // counter--; it doesn't touch the CARRY, so both decrement and right shift are safe to do ahead of time    
    cbi  0x5, 2      // PB2 = 0
    nop           
    nop 
    sbi  0x5, 2      // PB2 = 1 (each bit is pushed twice)   
    nop
    nop
    nop
    breq _neo_end    // if (0==counter) then end this loop (the end of the loop will do CBI anyway, so it's safe to do it ahead of time)
    nop 
    cbi  0x5, 2      // PB2 = 0 (but instead of timing NOPs do some decisions)
    brcc _bit_low    // if the carry from the shifted bit from the color was 0 then go to the low bit branch
    rjmp _bit_high   // otherwise repeat this high bit branch
                     
  _bit_low:
    sbi  0x5, 2      // PB2 = 1
    nop              // Timing NOP
    cbi  0x5, 2      // PB2 = 0
    nop 
    nop 
    lsr  r17         // shift the color bits again, sooner than it's needed (because later there will be no time) 
    ror  r16
    dec  r31         // counter--; it doesn't touch the CARRY, so both decrement and right shift are safe to do ahead of time
    sbi  0x5, 2      // PB2 = 1 (each bit is pushed twice)   
    nop              // Timing NOP
    cbi  0x5, 2      // PB2 = 0
    nop
    nop 
    breq _neo_end    // if (0==counter) then end this loop
    brcc _bit_low    // if the carry from the shifted bit from the color was 0 then repeat this low bit branch
    rjmp _bit_high   // otherwise go to the high bit branch
      
  _neo_end:      
    cbi  0x5,2       // PB2 = 0 as it might not have been set yet
                     // leave it at 0 as that will send the WS2812B chip a 'reset' command which will start displaying the color
                    
    sei              // enable IRQs
  #endasm
}


