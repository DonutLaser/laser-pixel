#if !defined (PIXEL_H)
#define PIXEL_H

#include "constants.h"

#include "../third_party/gui_math.h"
#include "../third_party/gui_resources.h"

// Forward declarations
struct gui_window;

enum Tool { T_DRAW, T_ERASE, T_SELECT, T_MOVE, T_COPY, T_PASTE }; 

struct mouse_drag {
	v2 origin;
	v2 offset;
	bool in_progress;
};

struct pixel_input {
	v2 mouse_pos;
	bool lmb_down;
	bool lmb_up;
};

struct pixel_app {
	gui_image icons[ICON_COUNT];
	int grid[GRID_TILE_COUNT_X][GRID_TILE_COUNT_Y];
	int selection_grid[GRID_TILE_COUNT_X][GRID_TILE_COUNT_Y];
	int clipboard_grid[GRID_TILE_COUNT_X][GRID_TILE_COUNT_Y];
	mouse_drag move; 
	unsigned color_index;
	Tool tool;
	bool tiles_selected;
	bool paste_executed;
};

void pixel_init (gui_window window, void* memory);
void pixel_update (void* memory, pixel_input input, gui_window window);

#endif