#include "pixel.h"

#include "pixel_gl.h"
#include "constants.h"
#include "../third_party/gui_io.h"

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
	int x_start = CONTROLS_OFFSET;

	rect first_frame_rect = make_rect (x_start, OUTER_MARGIN, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	x_start += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect prev_frame_rect = make_rect (x_start, OUTER_MARGIN, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	x_start += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect play_button_rect = make_rect (x_start, OUTER_MARGIN, PLAY_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	x_start += PLAY_BUTTON_WIDTH + INNER_MARGIN;

	rect next_frame_rect = make_rect (x_start, OUTER_MARGIN, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	x_start += FRAME_BUTTON_WIDTH + INNER_MARGIN;

	rect last_frame_rect = make_rect (x_start, OUTER_MARGIN, FRAME_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
	x_start += FRAME_BUTTON_WIDTH + INNER_MARGIN;

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
	v2 start_pos = make_v2 (OUTER_MARGIN, OUTER_MARGIN + SMALL_BUTTON_HEIGHT + INNER_MARGIN);

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
	v2 start_pos = make_v2 (OUTER_MARGIN + GRID_OUTLINE + GRID_TILE_SIZE * GRID_TILE_COUNT_X + GRID_OUTLINE + INNER_MARGIN,
							OUTER_MARGIN + SMALL_BUTTON_HEIGHT + INNER_MARGIN);

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

}

static void draw_buttons (pixel_input input) {

}

void pixel_init () {
	gl_init ();
}

void pixel_update (pixel_input input) {
	draw_controls (input);
	draw_grid (input);
	draw_colors (input);
}