/* Continuity Tester
 
   David Johnson-Davies - www.technoblogy.com - 18th November 2017
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <avr/sleep.h>
#include <avr/power.h>

const int LED = 2;
const int Reference = 0;                  // AIN0
const int Probe = 1;                      // AIN1
const int Speaker = 4;
const unsigned long Timeout = (unsigned long)60*1000; // One minute
volatile unsigned long Time;

// Pin change interrupt service routine - resets sleep timer
ISR (PCINT0_vect) {
  Time = millis();
}

void Beep () {
  TCCR1 = TCCR1 | 3;                      // Counter = clock/4
} 

void NoBeep () {
  TCCR1 = TCCR1 & ~3;                     // Counter stopped
}

void setup () {
  pinMode(Reference, INPUT_PULLUP);
  pinMode(Probe, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(Speaker, OUTPUT);
  pinMode(3, OUTPUT);                     // Don't leave floating
  // Setup beep
  TCCR1 = 1<<CTC1 | 0<<COM1A0 | 0<<CS10;  // CTC mode, counter stopped
  GTCCR = 1<<COM1B0;                      // Toggle OC1B (PB4)
  OCR1C = 119;                            // Plays 1042Hz (C6)
  // Pin-change interrupt
  PCMSK = 1<<Probe;                       // Pin change interrupt on Probe
  GIMSK = GIMSK | 1<<PCIE;                // Enable pin-change interrupt
  // Power saving
  ADCSRA &= ~(1<<ADEN);                   // Disable ADC to save power
  PRR = 1<<PRUSI | 1<<PRADC;              // Turn off unused clocks
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // Start running
  digitalWrite(LED, true);
  Time = millis();
}

void loop() {
  bool Sense = ACSR>>ACO & 1;
  if (Sense) Beep(); else NoBeep();
  // Go to sleep?
  if (millis() - Time > Timeout) {
    digitalWrite(LED, false);             // LED off
    pinMode(Reference, INPUT);            // Turn off pullup to save power
    sleep_enable();
    sleep_cpu();
    // Carry on here when we wake up
    pinMode(Reference, INPUT_PULLUP);
    digitalWrite(LED, true);              // LED on
  }
}


