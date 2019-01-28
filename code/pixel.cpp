#include "pixel.h"

#include "pixel_gl.h"
#include "constants.h"

#include "../third_party/gui_io.h"
#include "../third_party/gui_window.h"

static bool draw_button (rect r, pixel_input input) {
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

	return result;
}

static void draw_controls (pixel_input input) {
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

	if (draw_button (first_frame_rect, input))
		io_log ("Go to the first frame");
	if (draw_button (prev_frame_rect, input))
		io_log ("Go to the previous frame");
	if (draw_button (play_button_rect, input))
		io_log ("Play");
	if (draw_button (next_frame_rect, input))
		io_log ("Go to the next frame");
	if (draw_button (last_frame_rect, input))
		io_log ("Go to the last frame");
}

static void draw_grid (pixel_input input) {
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
			tile_color = tile_colors[(x + offset) % 2];

			tile_rect.x = start_pos.x + x * GRID_TILE_SIZE;
			tile_rect.y = start_pos.y + y * GRID_TILE_SIZE;
			gl_draw_rect (tile_rect, tile_color);

		}
	}
}

static void draw_colors (pixel_input input) {
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
			gl_draw_rect (tile_rect, tile_color);
		}
	}
}

static void draw_tools (pixel_input input) {
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

	if (draw_button (draw_rect, input))
		io_log ("Draw Tool");
	if (draw_button (erase_rect, input))
		io_log ("Erase Tool");
	if (draw_button (select_rect, input))
		io_log ("Select Tool");
	if (draw_button (move_rect, input))
		io_log ("Move Tool");
	if (draw_button (copy_rect, input))
		io_log ("Copy Tool");
	if (draw_button (paste_rect, input))
		io_log ("Paste Tool");
	if (draw_button (speed_rect, input))
		io_log ("Speed Tool");
}

static void draw_buttons (pixel_input input) {
	v2 start_pos = make_v2 (BUTTONS_POSITION);

	rect clear_rect = make_rect (start_pos, CLEAR_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += CLEAR_BUTTON_WIDTH + INNER_MARGIN;

	rect save_rect = make_rect (start_pos, SAVE_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += SAVE_BUTTON_WIDTH + INNER_MARGIN;

	rect load_rect = make_rect (start_pos, LOAD_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += LOAD_BUTTON_WIDTH + INNER_MARGIN;

	rect export_rect = make_rect (start_pos, EXPORT_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	start_pos.x += EXPORT_BUTTON_WIDTH + INNER_MARGIN;

	if (draw_button (clear_rect, input))
		io_log ("Clear frame");
	if (draw_button (save_rect, input))
		io_log ("Save animation");
	if (draw_button (load_rect, input))
		io_log ("Load animation");
	if (draw_button (export_rect, input))
		io_log ("Export animation");
}

void pixel_init (gui_window window) {
	gl_init (window);
}

void pixel_update (pixel_input input) {
	draw_controls (input);
	draw_grid (input);
	draw_colors (input);
	draw_tools (input);
	draw_buttons (input);
}