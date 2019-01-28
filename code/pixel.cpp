#include "pixel.h"

#include "pixel_gl.h"

#include "../third_party/gui_io.h"
#include "../third_party/gui_window.h"
#include "../third_party/gui_resources.h"

enum Icon { ICO_FIRST_FRAME, ICO_PREV_FRAME, ICO_PLAY, ICO_PAUSE, ICO_NEXT_FRAME, ICO_LAST_FRAME,
			ICO_DRAW, ICO_ERASE, ICO_SELECT, ICO_MOVE, ICO_COPY, ICO_PASTE, ICO_CLEAR,
			ICO_SAVE, ICO_LOAD, ICO_EXPORT, ICO_FULL_SPEED, ICO_HALF_SPEED };

static void set_tool (pixel_app* app, Tool tool, const char* text, gui_window window) {
	app -> tool = tool;

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

static bool draw_drawable_rect (rect r, v4 color, pixel_input input) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	if (is_point_in_rect (r, input.mouse_pos)) {
		gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255));
		gl_draw_rect (selected_rect, color);

		return input.lmb_down;
	}

	gl_draw_rect (r, color);
	return false;
}

static void draw_controls (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (CONTROLS_POSITION);

	rect first_frame_rect = make_rect (start_pos, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect prev_frame_rect = make_rect (start_pos, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect play_button_rect = make_rect (start_pos, PLAY_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += PLAY_BUTTON_WIDTH + INNER_MARGIN;

	rect next_frame_rect = make_rect (start_pos, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect last_frame_rect = make_rect (start_pos, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	if (draw_button (first_frame_rect, input, app -> icons[(int)ICO_FIRST_FRAME]))
		io_log ("Go to the first frame");
	if (draw_button (prev_frame_rect, input, app -> icons[(int)ICO_PREV_FRAME]))
		io_log ("Go to the previous frame");
	if (draw_button (play_button_rect, input, app -> icons[(int)ICO_PLAY]))
		io_log ("Play");
	if (draw_button (next_frame_rect, input, app -> icons[(int)ICO_NEXT_FRAME]))
		io_log ("Go to the next frame");
	if (draw_button (last_frame_rect, input, app -> icons[(int)ICO_LAST_FRAME]))
		io_log ("Go to the last frame");
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
			if (draw_drawable_rect (tile_rect, tile_color, input))
				app -> grid[y][x] = app -> color_index;
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

	rect draw_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos.x += LARGE_BUTTON_WIDTH + INNER_MARGIN;

	rect erase_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos.x += LARGE_BUTTON_WIDTH + INNER_MARGIN;

	rect select_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos.x += LARGE_BUTTON_WIDTH + INNER_MARGIN;

	rect move_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos = make_v2 (TOOLS_POSITION);
	start_pos.y += LARGE_BUTTON_HEIGHT + INNER_MARGIN;

	rect copy_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos.x += LARGE_BUTTON_WIDTH + INNER_MARGIN;

	rect paste_rect = make_rect (start_pos, LARGE_BUTTON_WIDTH, LARGE_BUTTON_HEIGHT);
	start_pos.x += LARGE_BUTTON_WIDTH + INNER_MARGIN;

	rect speed_rect = make_rect (start_pos, SPEED_WIDTH, LARGE_BUTTON_HEIGHT);

	if (draw_button (draw_rect, input, app -> icons[(int)ICO_DRAW]))
		set_tool (app, T_DRAW, "Draw Tool", window);
	if (draw_button (erase_rect, input, app -> icons[(int)ICO_ERASE]))
		set_tool (app, T_ERASE, "Erase Tool", window);
	if (draw_button (select_rect, input, app -> icons[(int)ICO_SELECT]))
		set_tool (app, T_SELECT, "Select Tool", window);
	if (draw_button (move_rect, input, app -> icons[(int)ICO_MOVE]))
		set_tool (app, T_MOVE, "Move Tool", window);
	if (draw_button (copy_rect, input, app -> icons[(int)ICO_COPY]))
		io_log ("Copy");
	if (draw_button (paste_rect, input, app -> icons[(int)ICO_PASTE]))
		io_log ("Paste");
	if (draw_button (speed_rect, input, app -> icons[(int)ICO_FULL_SPEED]))
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

	if (draw_button (clear_rect, input, app -> icons[(int)ICO_CLEAR]))
		io_log ("Clear frame");
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

	gl_init (window);
}

void pixel_update (void* memory, pixel_input input, gui_window window) {
	pixel_app* app = (pixel_app*)memory;
	draw_controls (app, input);
	draw_frame (app, input);
	draw_colors (app, input);
	draw_tools (app, input, window);
	draw_buttons (app, input);
}