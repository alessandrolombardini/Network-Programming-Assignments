#include "Timer.h"
#include "Arduino.h"

volatile bool timerFlag;

ISR(TIMER1_COMPA_vect){
  timerFlag = true;
}

Timer::Timer(){
  timerFlag = false;  
}

/* Period in ms */
void Timer::setupPeriod(int period){
  
  // Disabling interrupt
  cli();

  TCCR1A = 0; // Set entire TCCR1A register to 0
  TCCR1B = 0; // Same for TCCR1B
  TCNT1  = 0; // Initialize counter value to 0
  
  /* 
   * Set compare match register
   * 
   * OCR1A = (16*2^20) / (100*PRESCALER) - 1 (must be < 65536)
   *
   * Assuming a prescaler = 1024 => OCR1A = (16*2^10)* period/1000 (being in ms) 
   */
  OCR1A = 16.384*period; 
  // Turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 for 8 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Enabling interrupt
  sei();
}

void Timer::waitForNextTick(){
  /* Wait for timer signal */
  while (!timerFlag){}
  timerFlag = false;
}
