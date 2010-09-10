#define F_CPU 1000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <util/delay.h>

#define QUARTERSEC _BV(WDP2)
#define HALFSEC (_BV(WDP2) | _BV(WDP0))
#define TWOSECS (_BV(WDP2) | _BV(WDP1) | _BV(WDP0))
#define EIGHTSECS (_BV(WDP3) | _BV(WDP0))

inline void wdt_sleep(int);
inline void wdt_stop(void);
void power_idle(void);
void power_down(void);

inline void wdt_sleep(int mask) {
  WDTCR = _BV(WDIE) | mask; // Enable watchdog interrupt 
}

inline void wdt_stop() {
  WDTCR = _BV(WDCE);
  WDTCR = 0;
}

void power_idle() {
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_cpu();
  sleep_disable();
}

void power_down() {
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
  sleep_disable();
}
  

int main(void) {
  MCUSR = 0; // Reset watchdog status
  wdt_stop();
  power_all_disable();
  
  DDRB = _BV(PORTB0);
  sei();
  for(;;) {
    PORTB = _BV(PORTB0);
    wdt_sleep(HALFSEC);
    power_down();

    PORTB = 0;
    wdt_sleep(TWOSECS);
    power_down();
  }

  return 0;
}

ISR(WDT_vect) {
}
