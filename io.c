#include "vibrator.h"

inline void turn_on(output_t *output) {
  if (output->brightness < 255 && output->has_timer) {
    power_timer0_enable();
    TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM mode, inverting mode
    TCCR0B = _BV(CS00); // No prescaling
    OCR0A = output->brightness;
  } else {
    PORTB |= _BV(output->port);
  }
  output->state = ON;
}

inline void turn_off(output_t *output) {
  if (output->has_timer) {
    TCCR0A = 0;
    TCCR0B = 0;
    power_timer0_disable();
  }
  PORTB &= ~(_BV(output->port));
  output->state = OFF;
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

void set_output(output_t* output, uint16_t on_time, uint16_t off_time, uint16_t delay, uint8_t times, uint8_t brightness, bool_t repeat) {
  output->on_time = on_time;
  output->off_time = off_time;
  output->delay = delay;
  output->times = times;
  output->brightness = brightness;
  if (repeat) {
    output->repeat = times;
  } else {
    output->repeat = 0;
  }
  output->count_from = 0;
  output->state = OFF;
}

void handle_output(output_t *output) {
  if (output->times == 0) {
    turn_off(output);
    output->state = OFF;

    if (output->repeat) {
      output->times = output->repeat;
      output->count_from = counter;
      output->state = WAITING;
    } else {
      output->times = -1;
    }
  } 

  if (output->times > 0) {
    uint32_t elapsed = counter - output->count_from;

    switch (output->state) {
    case OFF:
      if (output->count_from == 0 || elapsed > output->off_time) {
        output->count_from = counter;
        turn_on(output);
      }
      break;
    case ON:
      if (elapsed > output->on_time) {
        output->times--;
        turn_off(output);
        output->count_from = counter;
      }
      break;
    case WAITING:
      if (elapsed > output->delay) {
        output->count_from = counter;
        turn_on(output);
      }
      break;
    }
  }
}
