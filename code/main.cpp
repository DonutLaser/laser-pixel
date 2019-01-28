#include "../third_party/gui_window.h"
#include "../third_party/gui_io.h"

#include "pixel.h"
#include "constants.h"

#if 1
#define GUI_DEBUG
#endif

GUI_MAIN {
#if defined (GUI_DEBUG)
	wnd_create_console ();
#endif

	gui_window window = wnd_create ("Pixel Playground", 419, 339, false);
	wnd_set_style (window, S_FIXEDSIZE);
	window.bg_color = make_color (BACKGROUND_COLOR, 255);

	pixel_init (window);

	pixel_input input;
	while (wnd_update (&window)) {
		input.mouse_pos = input_get_mouse_position (window);
		input.lmb_down = input_is_mb_pressed (window, M_LEFT);
		input.lmb_up = input_is_mb_up (window, M_LEFT);

		pixel_update (input);

		wnd_swap_buffers (window);
	}

	wnd_close ();
	return 0;
}