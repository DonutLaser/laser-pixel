#include "pixel.h"

#include "pixel_gl.h"

static void draw_grid (pixel_input input) {

}

static void draw_colors (pixel_input input) {
	gl_draw_rect (make_v4 (10, 10, 256, 256), make_v4 (0, 0, 0, 1));
}

static void draw_buttons (pixel_input input) {

}

void pixel_init () {
	gl_init ();
}

void pixel_update (pixel_input input) {
	draw_grid (input);
	draw_colors (input);
	draw_buttons (input);
}