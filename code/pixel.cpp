#include "pixel.h"

#include "pixel_gl.h"

#define COLOR_BTN_DEFAULT 	0.5f, 0.5f, 0.5f, 1.0f 
#define COLOR_BTN_HOVER 	0.4f, 0.4f, 0.4f, 1.0f
#define COLOR_BTN_ACTIVE 	0.2f, 0.2f, 0.2f, 1.0f

static bool draw_button (rect rect, v4 default_color, v4 hover_color, v4 active_color, 
						 pixel_input input) {
	v4 actual_color = default_color;
	if (is_point_in_rect (rect, input.mouse_pos)) {
		actual_color = hover_color;
		if (input.lmb_down)
			actual_color = active_color;
	}

	gl_draw_rect (rect, actual_color);

	return true;
}

static void draw_grid (pixel_input input) {

}

static void draw_colors (pixel_input input) {

}

static void draw_buttons (pixel_input input) {
	draw_button (make_rect (10, 10, 255, 64), make_v4 (COLOR_BTN_DEFAULT),
				 make_v4 (COLOR_BTN_HOVER), make_v4 (COLOR_BTN_ACTIVE), input);
}

void pixel_init () {
	gl_init ();
}

void pixel_update (pixel_input input) {
	draw_grid (input);
	draw_colors (input);
	draw_buttons (input);
}