/*******************************************************
Requires CodeVisionAVR C Compiler

Chip type               : ATmega88PA
AVR Core Clock frequency: 8 MHz

Memory model            : Small
Data Stack size         : 128 bytes
*******************************************************/

#include <mega88a.h>    // AVR Mega88 PA
#include <twi.h>        // TWI functions (I2C)
#include <ds3231_twi.h> // DS3231 Real Time Clock functions for TWI(I2C)
#include <sleep.h>      // Power managment
#include <stdint.h>     // `uint8_t` instead `unsigned char` and `uint16_t` instead `unsigned int`

#include "vfd.h"
#include "main.h" 
#include "neopixel.h"

#define PRESS_TO_SET_TIME 60 // 60 * 0.033s  = 2.0s press to trigger the 'set time'
#define PRESS_SHORT       15 // 15 * 0.033s  = 0.5s for short press


volatile uint8_t buttonPressed = 0; // Counter how long the WAKE-UP button is pressed (2Hz counter)
volatile uint8_t timeStale     = 0; // Flag to force 1Hz update from RTC to get correct time
volatile uint8_t systick       = 0; // 30Hz systick counter



// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
  systick = (systick + 1) % 30;    // 30Hz tick counter                                    
  if (0 == systick) timeStale = 1; // 1Hz flag to force the RTC update
    
  stayAwake = (stayAwake) ? stayAwake-1 : 0; // Countdown to 0               
                
  // Count how long the WAKE-UP button is pressed
  if (PIND & (1<<PORTD2)) {
    buttonPressed = 0;
  } else {                                    
    if (buttonPressed < 255) {
      // At certain point we do not need to count any further, just do not overflow
      buttonPressed++;
    } 
  }

}


// Pin change 16-23 interrupt service routine
// filtered to PCINT18/PD2 pin -> level changed on WAKE-UP button
interrupt [PC_INT2] void pin_change_isr2(void) {
  #asm("cli")                     // Globally disable interrupts
  PORTD         |= (1<<PORTD2);   // Go into internal pull up mode(~30k) to charge the pin up
  buttonPressed  = 0;             // Button state changed, start counting from scratch
  PORTD         &= ~(1<<PORTD2);  // Go back to a tri-state mode which is externally pulled up (~1M) 
  stayAwake      = SLEEP_TIMEOUT; // Pressing or lifting the button will keep us awake
  PCIFR         |= (1<<PCIF2);    // Clear pending IRQ caused by the pin discharge to 0 -> and pull up to 1
  #asm("sei")                     // Globally enable interrupts    
}


// Set all the internal peripherals into a good known state:
void systemPeripheralsSetup() {
  // Crystal Oscillator division factor: 1
  #pragma optsize-
  CLKPR=(1<<CLKPCE);
  CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif
                   
  // -------- Port B initialization --------  
  //     In            In            Out           In            Out           Out           Out           In
  DDRB= (0<<DDB7)   | (0<<DDB6)   | (1<<DDB5)   | (0<<DDB4)   | (1<<DDB3)   | (1<<DDB2)   | (1<<DDB1)   | (0<<DDB0);        
  //     T             T             0             T             T             0             0             T 
  PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

  // --------  Port C initialization -------- 
  //     In            In            In            In            In            In            In
  DDRC= (0<<DDC6)   | (0<<DDC5)   | (0<<DDC4)   | (0<<DDC3)   | (0<<DDC2)   | (0<<DDC1)   | (0<<DDC0);
  //     T             T             T             T             T             T             T             
  PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);  
  
  // -------- Port D initialization --------
  //     Out           In            In            In            In            In            Out           In
  DDRD= (1<<DDD7)   | (0<<DDD6)   | (0<<DDD5)   | (0<<DDD4)   | (0<<DDD3)   | (0<<DDD2)   | (1<<DDD1)   | (0<<DDD0);
  //     0             T             T             T             T             T             0             T             
  PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Clock value: 7.813 kHz
  // Mode: Normal top=0xFF
  // OC0A output: Disconnected
  // OC0B output: Disconnected
  // Timer Period: 32.768 ms
  TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
  TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);
  TCNT0=0x00;
  OCR0A=0x00;
  OCR0B=0x00;


  // Timer/Counter 1 initialization
  // Clock source: System Clock
  // Clock value: Timer1 Stopped
  // Mode: Normal top=0xFFFF
  // OC1A output: Disconnected
  // OC1B output: Disconnected
  // Noise Canceler: Off
  // Input Capture on Falling Edge
  // Timer1 Overflow Interrupt: Off
  // Input Capture Interrupt: Off
  // Compare A Match Interrupt: Off
  // Compare B Match Interrupt: Off
  TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
  TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
  TCNT1H=0x00;
  TCNT1L=0x00;
  ICR1H=0x00;
  ICR1L=0x00;
  OCR1AH=0x00;
  OCR1AL=0x00;
  OCR1BH=0x00;
  OCR1BL=0x00;
  

  // Timer/Counter 2 initialization
  // Clock source: System Clock
  // Clock value: Timer2 Stopped
  // Mode: Normal top=0xFF
  // OC2A output: Disconnected
  // OC2B output: Disconnected
  ASSR=(0<<EXCLK) | (0<<AS2);
  TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
  TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
  TCNT2=0x00;
  OCR2A=0x00;
  OCR2B=0x00;

  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

  // Timer/Counter 1 Interrupt(s) initialization
  TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (0<<TOIE1);

  // Timer/Counter 2 Interrupt(s) initialization
  TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (0<<TOIE2);

  // External Interrupt(s) initialization
  // INT0: Off
  // INT1: Off
  // Interrupt on any change on pins PCINT0-7: Off
  // Interrupt on any change on pins PCINT8-14: Off
  // Interrupt on any change on pins PCINT16-23: On the PCINT18/PD2 is the WAKE-UP button
  EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
  EIMSK=(0<<INT1) | (0<<INT0);
  PCICR=(1<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);
  PCMSK2=(0<<PCINT23) | (0<<PCINT22) | (0<<PCINT21) | (0<<PCINT20) | (0<<PCINT19) | (1<<PCINT18) | (0<<PCINT17) | (0<<PCINT16);
  PCIFR=(1<<PCIF2) | (0<<PCIF1) | (0<<PCIF0);

  // USART initialization
  // USART disabled
  UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (0<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);

  // Analog Comparator initialization
  // Analog Comparator: Off
  // The Analog Comparator's positive input is
  // connected to the AIN0 pin
  // The Analog Comparator's negative input is
  // connected to the AIN1 pin
  ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
  ADCSRB=(0<<ACME);
  // Digital input buffer on AIN0: On
  // Digital input buffer on AIN1: On
  DIDR1=(0<<AIN0D) | (0<<AIN1D);

  // ADC initialization
  // ADC disabled
  ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

  // SPI initialization to interact with MAX6920AWP and drive the VFD
  // Had to set the PB2,PB3 and PB5 as outputs (SS, CLK, MOSI) 
  // SPI Type: Master
  // SPI Clock Rate: 62.500 kHz (A /128 division, getting the slowest possible SCK clock)
  // SPI Clock Phase: Cycle Start
  // SPI Clock Polarity: Low
  // SPI Data Order: MSB First
  SPCR=(0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (1<<SPR1) | (1<<SPR0);
  SPSR=(0<<SPI2X);

  // TWI initialization to interact with DS3231M RTC I2C peripheral
  // Mode: TWI Master
  // Bit Rate: 100 kHz
  twi_master_init(100);
  
  #asm("sei")       // Globally enable interrupts
  sleep_enable();   // Enable power managment features

  // DS3231 Real Time Clock initialization for TWI
  // ~INT/SQW pin function: Disabled
  // 32 kHz pin output: Off
  rtc_init(DS3231_INT_SQW_OFF,0);
                   
  // Power on the VFD but make sure it doesn't display anything yet
  sendDataToVfd(0);  
  vfdOn();
}


// Something changed, counters need to start from scratch
void actionHappenedResetCounters(void) {
  buttonPressed = 0; 
  stayAwake     = SLEEP_TIMEOUT;
}


void setTimeStateMachine(uint8_t *hour, uint8_t *minute) {
  // state 0 normal operation - display clock
  // state 1 set hours
  // state 2 set minutes
  
  static uint8_t state  = 0;           

  switch (state) {
    
    case 1: // Set hours
      systick = 0; // Do not display the ':' dots when setting the hours  
      if (buttonPressed > PRESS_SHORT) {
        *hour = (*hour + 1) % 24;
        actionHappenedResetCounters(); 
      }      
    break;              
      
    case 2: // Set minutes
      systick = 15; // Constantly display the ':' dots when setting the minutes
      if (buttonPressed > PRESS_SHORT) {
        *minute = (*minute + 1) % 60;
        actionHappenedResetCounters(); 
      }
    break;
      
    default:
      // state 0 -> normal clock operation
        
      if (buttonPressed > PRESS_TO_SET_TIME) {
        // Go into the Set time state 
        state = 1;
        setNeopixel(NEOPIXEL_SET_HOURS_COLOR);
        actionHappenedResetCounters(); 
      } else {
        if (timeStale) { 
          // Do 1Hz RTC update of the exact time
          uint8_t second; // Seconds are not displayed and not used anywhere 
          rtc_get_time(hour, minute, &second);   
          timeStale = 0;             
        }
      }    
    break; // Not really needed here, but just for consistency sake      
  }
    
  if ( (state > 0) && (stayAwake < (SLEEP_TIMEOUT - PRESS_TO_SET_TIME)) ) { 
    // If in setting mode then after a few seconds of inactivity go to the next state automatically
    state++;                                                                                              
    setNeopixel(NEOPIXEL_SET_MINUTES_COLOR);
    actionHappenedResetCounters();      
  }
    
  if (state > 2) {
    // Reached the end, done setting the time, save it to the RTC chip and go to normal operation 
    rtc_set_time(*hour, *minute, 0);
    state = 0;  
    setNeopixel(NEOPIXEL_CLOCK_COLOR);    
    actionHappenedResetCounters();       
  }
}


void lowPowerAndWakingUp(uint8_t *hour, uint8_t *minute) {
  uint8_t second; // Seconds are not displayed and not used anywhere 
  
  if (0 == stayAwake) {           
    // Reached sleep timeout, going to power down state
    setNeopixel(NEOPIXEL_BLACK_COLOR);
    vfdOff();
    powerdown(); // External IRQ caused by WAKE-UP button can resume the CPU
                    
    // After waking up, get the current time as a lot of time could have passed
    rtc_get_time(hour, minute, &second);            
    setNeopixel(NEOPIXEL_CLOCK_COLOR);
    vfdOn();                            
  }
}


void main(void) {
  uint8_t hour   = 8;                    // On power up start with 8:00 time
  uint8_t minute = 0;
            
  systemPeripheralsSetup();              // Set all peripherals into a known state      
  rtc_set_time(hour, minute, 0);         // Set RTC clock to known time
  setNeopixel(NEOPIXEL_CLOCK_COLOR);  

  while (1) {                            // The super loop -> whole life of this watch                   
                           
    setTimeStateMachine(&hour, &minute); // Handles 'Set Time' functionality                               
    displayTime(hour, minute);           // The VFD needs constant refresh        
    lowPowerAndWakingUp(&hour, &minute); // Goes into low-power mode after a timeout 
  } 
  
}

