#include "vibrator.h"

#define DAY_POWER FULL_POWER
#define DAY_DURATION (TICKS_PER_SECOND / 2) // 1/4 second

#define WILD_POWER QUARTER_POWER
#define WILD_DURATION (TICKS_PER_SECOND / 4) // 1/4 second

#define MODE_CHANGE_TIME (TICKS_PER_SECOND / 4)

#define SETTINGS_TIMEOUT (TICKS_PER_SECOND * 10)

//#define ALARM_DELAY (TICKS_PER_SECOND * 60 * 60 * 7 + 60 * 20) // 7 hours 20 minutes
#define ALARM_DELAY (TICKS_PER_SECOND * 60 * 60 * 8 + 60 * 50) // 8 hours 50 minutes


#ifdef DEBUG

#define DAY_FREQUENCY (TICKS_PER_SECOND *  10) // 10 seconds

#define DILD_FREQUENCY (TICKS_PER_SECOND * 5) // 5 seconds
#define DILD_DELAY (TICKS_PER_SECOND * 15) // 15 seconds
#define DILD_DURATION (TICKS_PER_SECOND * 1) // 1 second
#define SLEEP_DELAY (TICKS_PER_SECOND * 10) // 10 seconds

#define WILD_FREQUENCY (TICKS_PER_SECOND * 15) // 45 seconds

#else // ELSE

#define DAY_FREQUENCY (TICKS_PER_SECOND * 60 * 10) // 10 minutes

#define SLEEP_DELAY (TICKS_PER_SECOND * 60 * 25) // 25 minutes
#define DILD_FREQUENCY (TICKS_PER_SECOND * 60 * 15) // 15 minutes
#define DILD_DURATION (TICKS_PER_SECOND * 2) // 2 seconds
//#define DILD_DELAY (TICKS_PER_SECOND * 60 * 60 * 5) // 5 hours
#define DILD_DELAY (TICKS_PER_SECOND * 60 * 150) // 2.5 hours
//#define DILD_DELAY (TICKS_PER_SECOND * 60 * 90) // 1.5 hours
//#define DILD_DELAY (TICKS_PER_SECOND * 60 * 30) // .5 hours

#define WILD_FREQUENCY (TICKS_PER_SECOND * 45) // 45 seconds

#endif


inline void switch_to_day() {
  mode = DAY;
  count_to = counter + DAY_FREQUENCY;
  set_output(&vibrator, MODE_CHANGE_TIME, MODE_CHANGE_TIME, 0, 1, FULL_POWER, false);
}

inline void switch_to_dild_waiting() {
  mode = DILD_WAITING;
  count_to = counter + DILD_DELAY;
  alarm_count_to = counter + ALARM_DELAY;
  set_output(&vibrator, MODE_CHANGE_TIME, MODE_CHANGE_TIME, 0, 2, FULL_POWER, false);
}

inline void switch_to_dild_active() {
  mode = DILD_ACTIVE;
  count_to = counter + SLEEP_DELAY;
  alarm_count_to = counter + ALARM_DELAY;
  set_output(&vibrator, MODE_CHANGE_TIME, MODE_CHANGE_TIME, 0, 3, FULL_POWER, false);
}

inline void switch_to_wild() {
  mode = WILD;
  count_to = counter + WILD_FREQUENCY;
  set_output(&vibrator, MODE_CHANGE_TIME, MODE_CHANGE_TIME, 0, 4, FULL_POWER, false);
}

inline void alarm_day() {
  count_to = counter + DAY_FREQUENCY;
  set_output(&vibrator, DAY_DURATION, DAY_DURATION, 0, 1, DAY_POWER, false);
}

inline void alarm_dild_waiting() {
  mode = DILD_ACTIVE;
  alarm_dild_active();
}

inline void alarm_dild_active() {
  count_to = counter + DILD_FREQUENCY;
  set_output(&vibrator, DILD_DURATION, DILD_DURATION, 0, 1, dild_power, false);
}

inline void alarm_wild() {
  /*
  count_to = counter + WILD_FREQUENCY;
  set_output(&vibrator, WILD_DURATION, WILD_DURATION, 0, 1, WILD_POWER, false);
  */

  // Temporary - put here to help find the right power / timing
  count_to = counter + 10 * TICKS_PER_SECOND;
  set_output(&vibrator, DILD_DURATION, DILD_DURATION, 0, 1, dild_power, false);
}


inline void init() {
  MCUSR = 0; // Reset watchdog status
  wdt_stop();
  power_all_disable();

  // Setup outputs
  vibrator.has_timer = true;
  vibrator.port = PORTB0;

  // Setup defaults
  dild_power = THIRD_POWER;

  CLKPR = _BV(CLKPCE); // Enable changing the clock prescaler
  CLKPR = _BV(CLKPS2) | _BV(CLKPS1); // Change the prescaler to 64 (125000Hz)

  DDRB = _BV(DDB0); // B0 is output (vibrator)
  PORTB = _BV(PORTB3) | _BV(PORTB4); // Pull up B3 and B4 (buttons)
  
  mode = DAY;
  count_to = counter + DAY_FREQUENCY;

  PORTB ^= _BV(PORTB0);
  _delay_ms(500);
  PORTB &= ~(_BV(PORTB0));
  _delay_ms(500);

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
  bool_t reallylongdoublepress = false;
  uint32_t doublepress_time = 0;
  uint8_t button_count = 0;

  for(;;) {
    update_button(&button1, PINB4);
    update_button(&button2, PINB3);

    if (button1.current == DOWN && button2.current == DOWN) {
      if (!doublepress)
        doublepress_time = counter;

      doublepress = true;

      if (doublepress_time + TICKS_PER_SECOND * 5 < counter) {
        if (!reallylongdoublepress) {
          reallylongdoublepress = true;
          set_output(&vibrator, TICKS_PER_SECOND / 8, TICKS_PER_SECOND / 8, 0, 2, FULL_POWER, false);
        }
      } else if (doublepress_time + TICKS_PER_SECOND * 2 < counter) {
        if (!longdoublepress) {
          longdoublepress = true;
          set_output(&vibrator, TICKS_PER_SECOND / 8, TICKS_PER_SECOND / 8, 0, 1, FULL_POWER, false);
        }
      }
    }

    if (doublepress) {
      if (button1.current == UP && button2.current == UP) {
        if (reallylongdoublepress) {
          main_mode = SETTING;
          button_count = 0;
        } else if (longdoublepress) {
          switch (mode) {
          case DAY: 
            switch_to_dild_waiting();
            break;
          case DILD_WAITING:
          case ALARM:
            switch_to_dild_active();
            break;
          case DILD_ACTIVE:
            switch_to_wild();
            break;
          case WILD:
            switch_to_day();
            break;
          }
        }

        doublepress = false;
        longdoublepress = false;
        reallylongdoublepress = false;
      }
    } else if (pressed(&button1) || pressed(&button2)) {
      if (main_mode == SETTING) {
        if (pressed(&button1)) {
          button_count++;
        } else {
          if (button_count > 0 && button_count < 9) {
            if (button_count == 8)
              dild_power = 255;
            else
              dild_power = button_count * 32;
            set_output(&vibrator, TICKS_PER_SECOND / 8, TICKS_PER_SECOND / 8, 0, button_count, dild_power, false);
          } else {
            set_output(&vibrator, TICKS_PER_SECOND * 3, 0, 0, 1, FULL_POWER, false);
          }
          main_mode = NORMAL;
        }
      } else {
        if (mode == DILD_ACTIVE || mode == DILD_WAITING) {
          uint32_t sleep_time = counter + SLEEP_DELAY;
          
          if (count_to < sleep_time)
            count_to = sleep_time;

          set_output(&vibrator, TICKS_PER_SECOND / 4, TICKS_PER_SECOND / 4, 0, 1, HALF_POWER, false);
        }
      }
    }

    if (main_mode == NORMAL) {
      if ((mode == DILD_ACTIVE) && (counter > alarm_count_to)) {
        mode = ALARM;
        set_output(&vibrator, 5, 30, 0, 50, FULL_POWER, true);
      } else if (counter > count_to) {
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
        case ALARM:
          break;
        }
      } 
    }

    button1.old = button1.current;
    button2.old = button2.current;

    handle_output(&vibrator);
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

