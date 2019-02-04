#if !defined (PIXEL_H)
#define PIXEL_H

#include "constants.h"

#include "../third_party/gui_math.h"
#include "../third_party/gui_resources.h"
#include "../third_party/gui_time.h"

// Forward declarations
struct gui_window;

enum tool { T_DRAW, T_ERASE, T_SELECT, T_MOVE, T_COPY, T_PASTE }; 
enum playback_speed { PS_FULL, PS_HALF };

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

struct frame {
	int grid[GRID_TILE_COUNT_Y][GRID_TILE_COUNT_X];
};

struct pixel_app {
	gui_image icons[ICON_COUNT];

	frame frames[MAX_FRAME_COUNT];
	int selection_grid[GRID_TILE_COUNT_Y][GRID_TILE_COUNT_X];
	int clipboard_grid[GRID_TILE_COUNT_Y][GRID_TILE_COUNT_X];

	mouse_drag move; 

	unsigned color_index;
	tool current_tool;

	int current_frame;
	unsigned frame_count;

	bool tiles_selected;
	bool paste_executed;

	bool is_playing;
	timer play_timer;
	playback_speed speed;
};

void pixel_init (gui_window window, void* memory);
void pixel_update (void* memory, pixel_input input, gui_window window);

#endif