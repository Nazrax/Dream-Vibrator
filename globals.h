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

typedef enum {false, true} bool_t;
typedef enum {UP, DOWN} button_state_t;
enum {DAY, DILD_WAITING, DILD_ACTIVE, WILD} mode;
enum {SLOW, FAST} timer;

typedef struct {
  button_state_t current, new, old;
  uint32_t time, down_time;
} button_t;

button_t button1, button2;

uint32_t counter;
uint32_t mode_time, off_time;
uint16_t ticks_per_wdt;

bool_t active;
bool_t calibrating;

#endif
