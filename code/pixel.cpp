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

}

static void draw_colors (pixel_input input) {

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
}