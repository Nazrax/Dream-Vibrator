#include "vibrator.h"

#define DAY_FREQUENCY (TICKS_PER_SECOND * 60 * 15)
#define DAY_DURATION (TICKS_PER_SECOND / 4)
#define DILD_DELAY (TICKS_PER_SECOND * 60 * 60 * 5)
#define DILD_DURATION (TICKS_PER_SECOND / 2)
#define DILD_FREQUENCY (TICKS_PER_SECOND * 60 * 10)
#define WILD_FREQUENCY (TICKS_PER_SECOND * 45)
#define WILD_DURATION (TICKS_PER_SECOND / 4)
#define ALARM_DELAY (TICKS_PER_SECOND * 60 * 60 * 7 + TICKS_PER_SECOND * 60 * 20) // 7 hours 20 minutes
#define SLEEP_DELAY (TICKS_PER_SECOND * 60 * 25)

inline void switch_to_day() {
  mode = DAY;
  mode_time = counter + DAY_FREQUENCY;
  off_time = counter + TICKS_PER_SECOND / 2;
  turn_on(FULL_POWER);
}

inline void switch_to_dild() {
  mode = DILD_WAITING;
  mode_time = counter + DILD_DELAY;
  off_time = counter + TICKS_PER_SECOND;
  alarm_time = counter + ALARM_DELAY;
  turn_on(FULL_POWER);
}

inline void switch_to_wild() {
  mode = WILD;
  mode_time = counter + WILD_FREQUENCY;
  off_time = counter + TICKS_PER_SECOND * 2;
  turn_on(FULL_POWER);
}

inline void alarm_day() {
  switch_to_day();
  off_time = counter + DAY_DURATION;
  turn_on(FULL_POWER);
}

inline void alarm_dild_waiting() {
  mode = DILD_ACTIVE;
  alarm_dild_active();
}

inline void alarm_dild_active() {
  mode_time = counter + DILD_FREQUENCY;
  off_time = counter + DILD_DURATION;
  turn_on(QUARTER_POWER);
}

inline void alarm_wild() {
  switch_to_wild();
  off_time = counter + WILD_DURATION;
  turn_on(FULL_POWER);
}


inline void init() {
  MCUSR = 0; // Reset watchdog status
  wdt_stop();
  power_all_disable();

  CLKPR = _BV(CLKPCE); // Enable changing the clock prescaler
  CLKPR = _BV(CLKPS2) | _BV(CLKPS1); // Change the prescaler to 64 (125000)

  DDRB = _BV(DDB0); // B0 is output (vibrator)
  PORTB = _BV(PORTB1) | _BV(PORTB2); // Pull up B1 and B2 (buttons)
  mode = DAY;

  PORTB ^= _BV(PORTB0);
  _delay_ms(1000);

  sei();

  //calibrate_wdt();
  fast_timer();
}

inline bool_t pressed(button_t *button) {
  return button->current == UP && button->old == DOWN;
}

int main(void) {
  init();
  bool_t doublepress = false;
  bool_t longdoublepress = false;
  uint32_t doublepress_time = 0;

  for(;;) {
    update_button(&button1, PINB2);
    update_button(&button2, PINB1);

    if (button1.current == DOWN && button2.current == DOWN) {
      if (!doublepress)
        doublepress_time = counter;

      doublepress = true;

      if (doublepress_time + TICKS_PER_SECOND * 2 < counter) {
        if (!longdoublepress) {
          longdoublepress = true;
          off_time = counter + TICKS_PER_SECOND / 8;
          turn_on(FULL_POWER);
        }
      }
    }

    if (doublepress) {
      if (button1.current == UP && button2.current == UP) {
        if (longdoublepress) {
          switch (mode) {
          case DAY: 
            switch_to_dild();
            break;
          case DILD_WAITING:
          case DILD_ACTIVE:
            switch_to_wild();
            break;
          case WILD:
            switch_to_day();
            break;
          }
          turn_on(FULL_POWER);
        }

        doublepress = false;
        longdoublepress = false;
      }
    } else if (pressed(&button1) || pressed(&button2)) {
      if (mode == DILD_ACTIVE || mode == DILD_WAITING) {
        uint32_t sleep_time = counter + SLEEP_DELAY;
        if (mode_time < sleep_time)
          mode_time = sleep_time;
        off_time = counter + TICKS_PER_SECOND / 4;
        turn_on(QUARTER_POWER);
      }
    }

    if ((mode == DILD_ACTIVE) && (counter > alarm_time)) {
      off_time = counter + TICKS_PER_SECOND * 5;
      alarm_time = counter + TICKS_PER_SECOND * 30;
      mode_time = counter + TICKS_PER_SECOND * 31; // Keep moving it past the alarm time
      turn_on(FULL_POWER);
    } else if (counter >= mode_time) {
      switch (mode) {
      case DAY: 
        alarm_day();
        break;
      case DILD_WAITING:
        alarm_dild_waiting();
        break;
      case DILD_ACTIVE:
        alarm_dild_active();
        break;
      case WILD:
        alarm_wild();
        break;
      }
    } else if (active && counter > off_time) {
      turn_off();
    }

    button1.old = button1.current;
    button2.old = button2.current;
  }

  return 0;
}

ISR(WDT_vect) {
  if (calibrating) 
    calibrating = false;
  else
    counter += ticks_per_wdt;
}

ISR(INT0_vect) {
}


ISR(TIMER1_COMPA_vect) {
  counter++;
}

