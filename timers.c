#include "vibrator.h"

void calibrate_wdt() {
  counter = 0;
  calibrating = true;
  fast_timer();
  wdt_start();
  while (calibrating)
    power_idle();
  ticks_per_wdt = counter;
  //slow_timer();
}

void slow_timer() {
  timer = SLOW;
  wdt_start();
  TCCR1 = 0; // stop timer1
  power_timer1_disable();
}

void fast_timer() {
  timer = FAST;
  wdt_stop();

  power_timer1_enable();

  TCNT1 = 0;
  TIMSK |= _BV(OCIE1A);
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS11) | _BV(CS10); // CTC mode; CLK / 1024 = 122 ticks/sec
  OCR1A = 1; // Fire interrupt 61 times / second
  OCR1C = 1; // CTC resets timer at the same time interrupt fires
}

inline void wdt_start() {
  WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0); // Enable watchdog interrupt at 8 seconds
}

inline void wdt_stop() {
  WDTCR = _BV(WDCE);
  WDTCR = 0;
}
