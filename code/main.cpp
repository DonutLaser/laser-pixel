#include "../third_party/gui_window.h"
#include "../third_party/gui_io.h"

#include "pixel.h"

#if 0
#define GUI_DEBUG
#endif

GUI_MAIN {
#if defined (GUI_DEBUG)
	wnd_create_console ();
#endif

	wnd_create ("Pixel Playground", 419, 339, false);
	wnd_set_style (S_FIXEDSIZE);

	pixel_init ();

	pixel_input input;
	while (wnd_update ()) {
		input.mouse_pos = input_get_mouse_position ();
		input.lmb_down = input_is_mb_pressed (M_LEFT);
		input.lmb_up = input_is_mb_up (M_LEFT);

		pixel_update (input);

		wnd_swap_buffers ();
	}

	wnd_close ();
	return 0;
}