#define F_CPU 125000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <util/delay.h>

#define TICKS_PER_SECOND 61

#define QUARTERSEC _BV(WDP2)
#define HALFSEC (_BV(WDP2) | _BV(WDP0))
#define TWOSECS (_BV(WDP2) | _BV(WDP1) | _BV(WDP0))
#define EIGHTSECS (_BV(WDP3) | _BV(WDP0))

typedef enum {false, true} bool_t;
typedef enum {UP, DOWN} button_state_t;
enum {DAY, DILD_WAITING, DILD_ACTIVE, WILD} mode;
enum {SLOW, FAST} timer;

typedef struct {
  button_state_t current, new, old;
  uint32_t time, down_time;
} button_t;

button_t button1, button2;

inline void wdt_sleep(int);
inline void wdt_stop(void);
void power_idle(void);
void power_down(void);
inline void turn_on(void);
inline void turn_off(void);
void update_button(button_t*, button_state_t);

void fast_timer(void);
void slow_timer(void);

uint32_t counter;
uint32_t off_time;
bool_t active;

void slow_timer() {
  timer = SLOW;
  TCCR1 = 0; // stop timer1
}

void fast_timer() {
  timer = FAST;
  wdt_stop();

  TCNT1 = 0;
  TIMSK |= _BV(OCIE1A);
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS11) | _BV(CS10); // CTC mode; CLK / 1024 = 122 ticks/sec
  OCR1A = 1; // Fire interrupt 61 times / second
  OCR1C = 1; // CTC resets timer at the same time interrupt fires
}

inline void wdt_sleep(int mask) {
  WDTCR = _BV(WDIE) | mask; // Enable watchdog interrupt 
}

inline void wdt_stop() {
  WDTCR = _BV(WDCE);
  WDTCR = 0;
}

void do_sleep(int mode) {
  sleep_enable();
  set_sleep_mode(mode);
  sleep_cpu();
  sleep_disable();
}

inline void power_idle() {
  do_sleep(SLEEP_MODE_IDLE);
}

inline void power_down() {
  do_sleep(SLEEP_MODE_PWR_DOWN);
}
  
inline void turn_on() {
  active = true;
  PORTB |= _BV(PORTB0);
}

inline void turn_off() {
  active = false;
  PORTB &= ~(_BV(PORTB0));
}

void update_button(button_t *button, uint8_t pin) {
  button_state_t state = !(PINB & _BV(pin));

  if (state != button->current) {
    if (button->time + 4 < counter) {
      button->old = button->current;
      button->current = state;
      button->time = counter;

      if (state == DOWN) 
        button->down_time = counter;

      fast_timer();
    }
  }
}

int main(void) {
  MCUSR = 0; // Reset watchdog status
  wdt_stop();
  //power_all_disable();

  CLKPR = _BV(CLKPCE); // Enable changing the clock prescaler
  CLKPR = _BV(CLKPS2) | _BV(CLKPS1); // Change the prescaler to 64 (125000)

  DDRB = _BV(DDB0); // B0 is output (vibrator)
  PORTB = _BV(PORTB1) | _BV(PORTB2); // Pull up B1 and B2 (buttons)
  mode = DAY;

  PORTB ^= _BV(PORTB0);
  _delay_ms(1000);


  sei();

  fast_timer();

  for(;;) {
    update_button(&button1, PINB2);
    update_button(&button2, PINB1);

    if (button1.current == UP && button1.old == DOWN) {
      turn_on();
    }

    if (button2.current == UP && button2.old == DOWN) {
      turn_off();
    }
      

    /*
    if (off_time < counter) {
      turn_off();
    } else {
      int sleep_time = 0;

      if (mode == DAY) {
        if (counter > 225) { // About half an hour
          sleep_time = HALFSEC;
        }
      } else if (mode == DILD_WAITING) {
        if (counter > 2250) { // About 5 hours
          mode = DILD_ACTIVE;
          sleep_time = HALFSEC;
        }
      } else if (mode == DILD_ACTIVE) {
        if (counter > 75) { // About 10 minutes
          sleep_time = HALFSEC;
        }
      } else if (mode == WILD) {
        if (counter > 7) { // About 1 minute
          sleep_time = QUARTERSEC;
        }
      }

      button.old = button.current;

      if (sleep) {
        counter = 0;
        turn_on();
        wdt_sleep(sleep_time);
        power_down();
      } else {
        wdt_sleep(EIGHTSECS);
        power_down();
      }
    }
    */
    button1.old = button1.current;
    button2.old = button2.current;
  }

  return 0;
}

ISR(WDT_vect) {
  counter += TICKS_PER_SECOND * 8;
}

ISR(INT0_vect) {
}


ISR(TIMER1_COMPA_vect) {
  counter++;
}

