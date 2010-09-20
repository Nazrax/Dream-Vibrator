#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_


inline void switch_to_day(void);
inline void switch_to_dild(void);
inline void switch_to_wild(void);

inline void alarm_day(void);
inline void alarm_dild_waiting(void);
inline void alarm_dild_active(void);
inline void alarm_wild(void);

inline void init(void);
void calibrate(void);

inline void wdt_start(void);
inline void wdt_stop(void);

inline void power_idle(void);
inline void power_down(void);

inline void turn_on(void);
inline void turn_off(void);

void update_button(button_t*, button_state_t);

void fast_timer(void);
void slow_timer(void);

void calibrate_wdt(void);

#endif
