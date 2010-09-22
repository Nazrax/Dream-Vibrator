#include "vibrator.h"

inline void turn_on(uint8_t brightness) {
  active = true;

  if (brightness < 255) {
    power_timer0_enable();
    TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM mode, inverting mode
    TCCR0B = _BV(CS00); // No prescaling
    OCR0A = brightness;
  } else {
    PORTB |= _BV(PORTB0);
  }
}

inline void turn_off() {
  active = false;
  if (TCCR0A) {
    TCCR0A = 0;
    TCCR0B = 0;
    power_timer0_disable();
  } else {
    PORTB &= ~(_BV(PORTB0));
  }
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
