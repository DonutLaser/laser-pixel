#if !defined (PIXEL_H)
#define PIXEL_H

#include "constants.h"

#include "../third_party/gui_math.h"

// Forward declarations
struct gui_window;

struct pixel_input {
	v2 mouse_pos;
	bool lmb_down;
	bool lmb_up;
};

struct pixel_app {
	int grid[GRID_TILE_COUNT_X][GRID_TILE_COUNT_Y];
	unsigned color_index;
};

void pixel_init (gui_window window, void* memory);
void pixel_update (void* memory, pixel_input input);

#endif