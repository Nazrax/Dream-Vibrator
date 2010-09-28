#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define F_CPU 125000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <util/delay.h>

#define TICKS_PER_SECOND 61L

#define FULL_POWER 255
#define HALF_POWER 128
#define THIRD_POWER 85
#define QUARTER_POWER 64

uint8_t dild_power;

typedef enum {false, true} bool_t;
typedef enum {UP, DOWN} button_state_t;
typedef enum {OFF, ON, WAITING} output_state_t;
enum {DAY, DILD_WAITING, DILD_ACTIVE, WILD, ALARM} mode;
enum {NORMAL, SETTING} main_mode;
enum {SLOW, FAST} timer;

typedef struct {
  button_state_t current, new, old;
  uint32_t time, down_time;
} button_t;

typedef struct {
  uint8_t port, brightness;
  int8_t times, repeat;
  uint16_t on_time, off_time, delay;
  bool_t has_timer;
  uint32_t count_from;
  output_state_t state;
} output_t;

button_t button1, button2;
output_t vibrator;

volatile uint32_t counter;
uint32_t count_to, alarm_count_to;
uint16_t ticks_per_wdt;

bool_t calibrating;

#endif
