/*******************************************************
Requires CodeVisionAVR C Compiler

Chip type               : ATmega88PA
AVR Core Clock frequency: 8 MHz

Memory model            : Small
Data Stack size         : 128 bytes
*******************************************************/

#include <mega88a.h>    // AVR Mega88 PA
#include <ds3231_twi.h> // DS3231 Real Time Clock functions for TWI(I2C)
#include <sleep.h>      // Power managment
#include <stdint.h>     // `uint8_t` instead `unsigned char` and `uint16_t` instead `unsigned int`

#include "main.h" 
#include "reset.h"
#include "vfd.h"
#include "neopixel.h"


volatile uint8_t buttonPressed  = 0; // Counter how long the WAKE-UP button is pressed (20Hz counter)
volatile bit     timeStale      = 0; // Flag to force 1Hz update from RTC to get correct time
volatile uint8_t systick        = 0; // 20Hz systick counter


// Timer1 output compare A interrupt service routine (a 20Hz systick)
interrupt [TIM1_COMPA] void timer1_compa_isr(void) {
  systick = (systick + 1) % SYSTICK_MAX;     // 20Hz tick counter                                    
  neopixelUpdate = 1;                        // Flag set true to update the Neopixel at systick frequency
  if (0 == systick) timeStale = 1;           // 1Hz flag to force the RTC update    
  stayAwake = (stayAwake) ? stayAwake-1 : 0; // Countdown to 0               
                
  // Count how long the WAKE-UP button is pressed
  if (PIND & (1<<PORTD2)) {
    // Is in pull-up state means the button is not pressed
    buttonPressed = 0;    
  } else {
                                                 
    // Button is pressed right now
    if (buttonPressed < 255) {
      // At certain point we do not need to count any further, just do not overflow
      buttonPressed++;
    } 
  }

}


// Pin change 16-23 interrupt service routine
// filtered to PCINT18/PD2 pin -> level changed on the WAKE-UP button
interrupt [PC_INT2] void pin_change_isr2(void) {
  #asm("cli")                     // Globally disable interrupts
  PORTD         |= (1<<PORTD2);   // Go into internal pull up mode(~30k) to charge the pin up
  buttonPressed  = 0;             // Button state changed, start counting from scratch
  PORTD         &= ~(1<<PORTD2);  // Go back to a tri-state mode which is externally pulled up (~1M) 
  stayAwake      = SLEEP_TIMEOUT; // Pressing or lifting the button will keep us awake
  PCIFR         |= (1<<PCIF2);    // Clear pending IRQ caused by the pin charge from possible 0 to 1
  #asm("sei")                     // Globally enable interrupts    
}


// Something changed, maybe a pressed button or a clock state changed. Counters need to start from scratch.
void actionHappenedResetCounters(void) {
  buttonPressed = 0; 
  stayAwake     = SLEEP_TIMEOUT;
}


// Handle all 3 states the clock can be in
void setTimeStateMachine(uint8_t *hour, uint8_t *minute) {
  // state 0 normal operation - display clock
  // state 1 set hours
  // state 2 set minutes
  
  static uint8_t state  = 0;           

  switch (state) {
    
    case 1:  // Set hours
      systick = 0; // Do not display the ':' dots when setting the hours  
      if (buttonPressed > PRESS_SHORT) {
        *hour = (*hour + 1) % 24;
        actionHappenedResetCounters(); 
      }      
    break;              
      
    case 2:  // Set minutes
      systick = SYSTICK_MAX/2; // Constantly display the ':' dots when setting the minutes
      if (buttonPressed > PRESS_SHORT) {
        *minute = (*minute + 1) % 60;
        actionHappenedResetCounters(); 
      }
    break;
      
    default: // state 0 -> normal clock operation        
      if (buttonPressed > PRESS_TO_SET_TIME) {
        // Pressed button for too long -> go into the 'Set time' states 
        state = 1;
        neopixelSetColor(NEOPIXEL_SET_HOURS_COLOR);
        actionHappenedResetCounters(); 
      } else {                                    
      
        // Nothing pressed, just display the clock
        if (timeStale) {  
          // Do 1Hz RTC update of the exact time
          uint8_t second; // Seconds are not displayed and not used anywhere 
          rtc_get_time(hour, minute, &second);   
          timeStale = 0;             
        }
      }    
    break; // Not needed here, but just for consistency sake      
  }
    
  if ( (state > 0) && (stayAwake < (SLEEP_TIMEOUT - PRESS_TO_SET_TIME)) ) { 
    // If currently in any setting mode, then after a few seconds of inactivity go to the next state automatically
    state++;                                                                                              
    neopixelSetColor(NEOPIXEL_SET_MINUTES_COLOR);
    actionHappenedResetCounters();      
  }
    
  if (state > 2) {
    // Reached the end of state machine, done with setting the time, 
    // save the new time to the RTC chip and go to normal operation 
    rtc_set_time(*hour, *minute, 0);
    state = 0;  
    neopixelSetColor(NEOPIXEL_CLOCK_COLOR);    
    actionHappenedResetCounters();       
  }
}


void lowPowerAndWakingUp(uint8_t *hour, uint8_t *minute) {
  uint8_t second; // Seconds are not displayed and not used anywhere 
  
  if (0 == stayAwake) {           
    // Reached sleep timeout, going to power down state
    neopixelSetColor(NEOPIXEL_BLACK_COLOR);
    vfdOff();
    powerdown(); // External IRQ caused by the WAKE-UP button can resume the CPU
                    
    // After waking up, get the current time as a lot of time could have passed
    rtc_get_time(hour, minute, &second);            
    neopixelFadeCountDown = NEOPIXEL_START_FADE; // Start Neopixel's fade from black to red 
    vfdOn();                            
  }
}


void main(void) {
  uint8_t hour   = 8;                          // On power up start with 8:00 time
  uint8_t minute = 0;
            
  systemPeripheralsSetup();                    // Set all peripherals into a known state      
  rtc_set_time(hour, minute, 0);               // Set RTC clock to a known time  
  neopixelFadeCountDown = NEOPIXEL_START_FADE; // Start Neopixel's fade from black to red

  while (1) {                                  // The super loop -> whole life of this watch                   
                           
    setTimeStateMachine(&hour, &minute);       // Handles 'Set Time' functionality                               
    displayTime(hour, minute);                 // The VFD needs constant refresh 
    neopixelFadeHandler();                     // Updates the Neopixel color when the fade is enabled       
    lowPowerAndWakingUp(&hour, &minute);       // Goes into low-power mode after a timeout 
  } 
  
}

