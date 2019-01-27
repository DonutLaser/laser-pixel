#include "pixel.h"

#include "pixel_gl.h"
#include "constants.h"
#include "../third_party/gui_io.h"

static bool draw_button (rect rect, v4 default_color, v4 hover_color, v4 active_color, 
						 pixel_input input) {
	v4 actual_color = default_color;
	if (is_point_in_rect (rect, input.mouse_pos)) {
		actual_color = hover_color;
		if (input.lmb_down)
			actual_color = active_color;
	}

	gl_draw_rect (rect, actual_color);

	return input.lmb_up;
}

static void draw_controls (pixel_input input) {

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
	
}