#include "vibrator.h"

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
  
