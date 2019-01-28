#if !defined (PIXEL_H)
#define PIXEL_H

#include "../third_party/gui_math.h"

// Forward declarations
struct gui_window;

struct pixel_input {
	v2 mouse_pos;
	bool lmb_down;
	bool lmb_up;
};

void pixel_init (gui_window window);
void pixel_update (pixel_input input);

#endif