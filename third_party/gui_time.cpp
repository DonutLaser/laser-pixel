#include "gui_time.h"

#include <windows.h>

void timer_reset (timer* t) {
	t -> start = GetTickCount ();
}

unsigned timer_get_value (timer t) {
	return GetTickCount () - t.start;
}