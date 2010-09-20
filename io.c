#include "vibrator.h"

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
