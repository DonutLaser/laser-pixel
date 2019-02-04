#if !defined (GUI_TIME_H)
#define GUI_TIME_H

struct timer {
	unsigned start;
	unsigned target_miliseconds;
};

void timer_reset (timer* t);
unsigned timer_get_value (timer t);

#endif