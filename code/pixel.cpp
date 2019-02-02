#include "pixel.h"

#include "pixel_gl.h"

#include "../third_party/gui_io.h"
#include "../third_party/gui_window.h"
#include "../third_party/gui_resources.h"

enum Icon { ICO_FIRST_FRAME, ICO_PREV_FRAME, ICO_PLAY, ICO_PAUSE, ICO_NEXT_FRAME, ICO_LAST_FRAME,
			ICO_DRAW, ICO_ERASE, ICO_SELECT, ICO_MOVE, ICO_COPY, ICO_PASTE, ICO_CLEAR,
			ICO_SAVE, ICO_LOAD, ICO_EXPORT, ICO_FULL_SPEED, ICO_HALF_SPEED };

static void clear_selection (pixel_app* app) {
	if (!app -> tiles_selected)
		return;

	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
			app -> selection_grid[y][x] = -1;
	}

	app -> tiles_selected = false;
}

static void set_tool (pixel_app* app, Tool tool, const char* text, gui_window window) {
	app -> tool = tool;

	if (app -> tool != T_MOVE)
		clear_selection (app);

	wnd_set_title (window, "Pixel Playground | %s |", text);
}

static bool draw_button (rect r, pixel_input input, gui_image icon) {
	bool result = false;
	v4 outline_color = make_color (OUTLINE_COLOR, 255);
	v4 actual_color = make_color (DEFAULT_BUTTON_COLOR, 255);

	if (is_point_in_rect (r, input.mouse_pos)) {
		outline_color = make_color (BUTTON_HOVER_OUTLINE_COLOR, 255);
		if (input.lmb_down)
			actual_color = make_color (BUTTON_ACTIVE_COLOR, 255);

		result = input.lmb_up;
	}

	rect actual_rect = r;
	actual_rect.x += DEFAULT_OUTLINE;
	actual_rect.y += DEFAULT_OUTLINE;
	actual_rect.width -= DEFAULT_OUTLINE * 2;
	actual_rect.height -= DEFAULT_OUTLINE * 2;

	gl_draw_rect (r, outline_color);
	gl_draw_rect (actual_rect, actual_color);
	gl_draw_image (r, make_color (255, 255, 255, 255), icon);

	return result;
}

static bool draw_selectable_rect (rect r, v4 color, pixel_input input, bool is_selected) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	if (is_selected) {
		gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
		gl_draw_rect (selected_rect, color);

		return false;
	}
	else if (is_point_in_rect (r, input.mouse_pos)) {
		gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
		gl_draw_rect (selected_rect, color);

		return input.lmb_up;
	}

	gl_draw_rect (r, color);
	return false;
}

static bool draw_drawable_rect (rect r, v4 color, pixel_input input, bool is_selected) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	if (is_selected) {
		gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
		gl_draw_rect (selected_rect, color);	

		return false;
	}
	else if (is_point_in_rect (r, input.mouse_pos)) {
		gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
		gl_draw_rect (selected_rect, color);

		return input.lmb_down;
	}

	gl_draw_rect (r, color);
	return false;
}

static void draw_selected_rect (rect r, v4 color) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
	gl_draw_rect (selected_rect, color);
}

static void draw_controls (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (CONTROLS_POSITION);

	unsigned widths[] = {
		FRAME_BUTTON_WIDTH,
		FRAME_BUTTON_WIDTH,
		PLAY_BUTTON_WIDTH,
		FRAME_BUTTON_WIDTH,
		FRAME_BUTTON_WIDTH
	};

	unsigned icons[] = {
		(int)ICO_FIRST_FRAME,
		(int)ICO_PREV_FRAME,
		(int)ICO_PLAY,
		(int)ICO_NEXT_FRAME,
		(int)ICO_LAST_FRAME
	};

	rect r = { };
	for (unsigned i = 0; i < 5; ++i) {
		r = make_rect (start_pos, (float)widths[i], SMALL_BUTTON_HEIGHT);
		start_pos.x += widths[i] + INNER_MARGIN;

		if (draw_button (r, input, app -> icons[icons[i]]))
			io_log ("Do Frame action");
	}
}

static void draw_frame (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (GRID_POSITION);

	rect outline_rect = make_rect (start_pos,
								   GRID_TILE_SIZE * GRID_TILE_COUNT_X + GRID_OUTLINE * 2, 
								   GRID_TILE_SIZE * GRID_TILE_COUNT_Y + GRID_OUTLINE * 2);
	gl_draw_rect (outline_rect, make_color (OUTLINE_COLOR, 255));

	start_pos.x += GRID_OUTLINE;
	start_pos.y += GRID_OUTLINE;

	rect tile_rect = make_rect (start_pos, GRID_TILE_SIZE, GRID_TILE_SIZE);

	v4 tile_colors[] = {
		make_color (GRID_TILE_LIGHT, 255),
		make_color (GRID_TILE_DARK, 255)	
	};
	v4 tile_color = tile_colors[0];
	unsigned offset = 0;
	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		offset = y % 2;

		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
			int index = app -> grid[y][x];
			if (index < 0)
				tile_color = tile_colors[(x + offset) % 2];
			else
				tile_color = colors[index];

			tile_rect.x = start_pos.x + x * GRID_TILE_SIZE;
			tile_rect.y = start_pos.y + y * GRID_TILE_SIZE;

			if (app -> tool == T_MOVE)
				input.mouse_pos = make_v2 (-1, -1);

			bool is_selected = app -> selection_grid[y][x] >= 0 && app -> tool != T_MOVE;

			if (draw_drawable_rect (tile_rect, tile_color, input, is_selected)) {
				if (app -> tool == T_DRAW) {
					app -> grid[y][x] = app -> color_index;
					clear_selection (app);
				}
				else if (app -> tool == T_ERASE) {
					app -> grid[y][x] = -1;
					clear_selection (app);
				}
				else if (app -> tool == T_SELECT) {
					app -> selection_grid[y][x] = app -> grid[y][x] >= 0 ? app -> grid[y][x] : -1;
					app -> tiles_selected = true;
				}
			}
		}
	}
}

static void draw_selected_pixels (pixel_app* app, pixel_input input) {
	if (input.lmb_down && !app -> move.in_progress) {
		v2 start_pos = make_v2 (GRID_POSITION);
		rect outline_rect = make_rect (start_pos,
								   	   GRID_TILE_SIZE * GRID_TILE_COUNT_X + GRID_OUTLINE * 2, 
								       GRID_TILE_SIZE * GRID_TILE_COUNT_Y + GRID_OUTLINE * 2);
		if (is_point_in_rect (outline_rect, input.mouse_pos)) {
			app -> move.origin = input.mouse_pos;
			app -> move.in_progress = true;

			for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
				for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
					if (app -> selection_grid[y][x] >= 0)
						app -> grid[y][x] = -1;
				}
			}
		}
	}
	else if (input.lmb_down && app -> move.in_progress) {
		app -> move.offset.x = (float)(((int)input.mouse_pos.x - (int)app -> move.origin.x) / GRID_TILE_SIZE);
		app -> move.offset.y = (float)(((int)input.mouse_pos.y - (int)app -> move.origin.y) / GRID_TILE_SIZE);
	}
	else if (input.lmb_up) {
		app -> move.in_progress = false;

		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
				if (app -> selection_grid[y][x] >= 0) {
					if (BETWEEN (y + (int)app -> move.offset.y, 0, GRID_TILE_COUNT_Y - 1) &&
						BETWEEN (x + (int)app -> move.offset.x, 0, GRID_TILE_COUNT_X - 1)) {
						app -> grid[y + (int)app -> move.offset.y][x + (int)app -> move.offset.x] = 
							app -> selection_grid[y][x];
					}
				}
			}
		}

		// Update the selection grid
		int updated_selection_grid[GRID_TILE_COUNT_Y][GRID_TILE_COUNT_X];
		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
				if (app -> selection_grid[y][x] >= 0) {
					if (BETWEEN (y + (int)app -> move.offset.y, 0, GRID_TILE_COUNT_Y - 1) &&
						BETWEEN (x + (int)app -> move.offset.x, 0, GRID_TILE_COUNT_X - 1)) {
						updated_selection_grid[y + (int)app -> move.offset.y][x + (int)app -> move.offset.x] = 
							app -> selection_grid[y][x];
					}
				}
			}
		}

		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
				if (updated_selection_grid[y][x] >= 0)
					app -> selection_grid[y][x] = updated_selection_grid[y][x];
				else
					app -> selection_grid[y][x] = -1;
			}
		}		

		app -> move.offset = make_v2 (0, 0);
	}

	v2 start_pos = make_v2 (GRID_POSITION);
	start_pos.x += GRID_OUTLINE + app -> move.offset.x * GRID_TILE_SIZE;
	start_pos.y += GRID_OUTLINE + app -> move.offset.y * GRID_TILE_SIZE;

	rect tile_rect = make_rect (start_pos, GRID_TILE_SIZE, GRID_TILE_SIZE);

	v4 transparent = make_color (0, 0, 0, 0);
	v4 tile_color = transparent;
	unsigned offset = 0;
	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		offset = y % 2;

		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
			int index = app -> selection_grid[y][x];
			if (index < 0)
				tile_color = transparent;
			else
				tile_color = colors[index];

			tile_rect.x = start_pos.x + x * GRID_TILE_SIZE;
			tile_rect.y = start_pos.y + y * GRID_TILE_SIZE;

 			if (app -> selection_grid[y][x] >= 0) 
 				draw_selected_rect (tile_rect, tile_color);
			else
				gl_draw_rect (tile_rect, tile_color);
		}
	}
}

static void draw_colors (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (COLORS_POSITION);

	rect outline_rect = make_rect (start_pos, 
								   COLOR_TILE_SIZE * COLOR_TILE_COUNT_X + GRID_OUTLINE * 2,
								   COLOR_TILE_SIZE * COLOR_TILE_COUNT_Y + GRID_OUTLINE * 2);
	gl_draw_rect (outline_rect, make_color (OUTLINE_COLOR, 255));

	start_pos.x += GRID_OUTLINE;
	start_pos.y += GRID_OUTLINE;

	rect tile_rect = make_rect (start_pos, COLOR_TILE_SIZE, COLOR_TILE_SIZE);

	v4 tile_color = colors[0];
	for (unsigned y = 0; y < COLOR_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < COLOR_TILE_COUNT_X; ++x) {
			tile_color = colors[x + y * COLOR_TILE_COUNT_X];

			tile_rect.x = start_pos.x + x * COLOR_TILE_SIZE;
			tile_rect.y = start_pos.y + y * COLOR_TILE_SIZE;

			if (draw_selectable_rect (tile_rect, tile_color, input, app -> color_index == x + y * COLOR_TILE_COUNT_X))
				app -> color_index = x + y * COLOR_TILE_COUNT_X;
		}
	}
}

static void draw_tools (pixel_app* app, pixel_input input, gui_window window) {
	v2 start_pos = make_v2 (TOOLS_POSITION);

	unsigned widths[] = { LARGE_BUTTON_WIDTH, SPEED_WIDTH };
	rect rects[7];

	for (unsigned i = 0; i < 7; ++i) {
		rects[i] = make_rect (start_pos, i < 6 ? (float)widths[0]: (float)widths[1], LARGE_BUTTON_HEIGHT);
		start_pos.x += widths[0] + INNER_MARGIN;

		if (i == 3) {
			start_pos = make_v2 (TOOLS_POSITION);
			start_pos.y += LARGE_BUTTON_HEIGHT + INNER_MARGIN;
		}
	}

	if (draw_button (rects[0], input, app -> icons[(int)ICO_DRAW]))
		set_tool (app, T_DRAW, "Draw Tool", window);
	if (draw_button (rects[1], input, app -> icons[(int)ICO_ERASE]))
		set_tool (app, T_ERASE, "Erase Tool", window);
	if (draw_button (rects[2], input, app -> icons[(int)ICO_SELECT]))
		set_tool (app, T_SELECT, "Select Tool", window);
	if (draw_button (rects[3], input, app -> icons[(int)ICO_MOVE]))
		set_tool (app, T_MOVE, "Move Tool", window);
	if (draw_button (rects[4], input, app -> icons[(int)ICO_COPY]))
		io_log ("Copy");
	if (draw_button (rects[5], input, app -> icons[(int)ICO_PASTE]))
		io_log ("Paste");
	if (draw_button (rects[6], input, app -> icons[(int)ICO_FULL_SPEED]))
		io_log ("Speed Tool");
}

static void draw_buttons (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (BUTTONS_POSITION);

	rect clear_rect = make_rect (start_pos, CLEAR_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += CLEAR_BUTTON_WIDTH + INNER_MARGIN;

	rect save_rect = make_rect (start_pos, SAVE_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += SAVE_BUTTON_WIDTH + INNER_MARGIN;

	rect load_rect = make_rect (start_pos, LOAD_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += LOAD_BUTTON_WIDTH + INNER_MARGIN;

	rect export_rect = make_rect (start_pos, EXPORT_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += EXPORT_BUTTON_WIDTH + INNER_MARGIN;

	if (draw_button (clear_rect, input, app -> icons[(int)ICO_CLEAR])) {
		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
				app -> grid[y][x] = -1;
		}
	}
	if (draw_button (save_rect, input, app -> icons[(int)ICO_SAVE]))
		io_log ("Save animation");
	if (draw_button (load_rect, input, app -> icons[(int)ICO_LOAD]))
		io_log ("Load animation");
	if (draw_button (export_rect, input, app -> icons[(int)ICO_EXPORT]))
		io_log ("Export animation");
}

void pixel_init (gui_window window, void* memory) {
	pixel_app* app = (pixel_app*)memory;
	app -> color_index = 0;

	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
			app -> grid[y][x] = -1;
	}

	for (unsigned i = 0; i < ICON_COUNT; ++i) {
		resources_load_image (icons[i], &app -> icons[(Icon)i]);
		gl_load_image (&app -> icons[(Icon)i]);
	}

	set_tool (app, T_DRAW, "Draw Tool", window);

	app -> move.in_progress = false;
	app -> move.offset = make_v2 (0, 0);

	app -> tiles_selected = false;

	gl_init (window);
}

void pixel_update (void* memory, pixel_input input, gui_window window) {
	pixel_app* app = (pixel_app*)memory;

	draw_controls (app, input);

	rect clip_rect = make_rect (make_v2 (GRID_POSITION),
								GRID_TILE_SIZE * GRID_TILE_COUNT_X,
								GRID_TILE_SIZE * GRID_TILE_COUNT_Y);
	clip_rect.x += GRID_OUTLINE;
	clip_rect.y += GRID_OUTLINE;
	gl_begin_clip_rect (wnd_get_client_size (window), clip_rect);

	draw_frame (app, input);
	if (app -> tool == T_MOVE)
		draw_selected_pixels (app, input);

	gl_end_clip_rect ();

	draw_colors (app, input);
	draw_tools (app, input, window);
	draw_buttons (app, input);
}