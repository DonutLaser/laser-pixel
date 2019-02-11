#include "pixel.h"

#include "pixel_gl.h"
#include "pixel_parser.h"

#include "../third_party/gui_io.h"
#include "../third_party/gui_window.h"
#include "../third_party/gui_string_buffer.h"

enum Icon { ICO_FIRST_FRAME, ICO_PREV_FRAME, ICO_PLAY, ICO_PAUSE, ICO_NEXT_FRAME, ICO_LAST_FRAME,
			ICO_DRAW, ICO_ERASE, ICO_SELECT, ICO_MOVE, ICO_COPY, ICO_PASTE, ICO_CLEAR,
			ICO_SAVE, ICO_LOAD, ICO_EXPORT, ICO_FULL_SPEED, ICO_HALF_SPEED };

enum change_frame_type { CF_FIRST, CF_PREV, CF_PLAY, CF_NEXT, CF_LAST };

static void clear_selection (pixel_app* app) {
	if (!app -> tiles_selected)
		return;

	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
			app -> selection_grid[y][x] = -1;
	}

	app -> tiles_selected = false;
}

static void copy_to_clipboard (pixel_app* app) {
	if (app -> tiles_selected) {
		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
				if (app -> selection_grid[y][x] >= 0)
					app -> clipboard_grid[y][x] = app -> selection_grid[y][x];
			}
		}
	}
	else {
		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
				if (app -> project.frames[app -> current_frame].grid[y][x] >= 0)
					app -> clipboard_grid[y][x] = app -> project.frames[app -> current_frame].grid[y][x];
			}
		}
	}
}

static void paste (pixel_app* app) {
	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
			if (app -> clipboard_grid[y][x] >= 0)
				app -> selection_grid[y][x] = app -> clipboard_grid[y][x];
			else
				app -> selection_grid[y][x] = -1;
		}
	}

	app -> paste_executed = true;
	app -> tiles_selected = true;
}

static void set_tool (pixel_app* app, tool t, const char* text, gui_window window) {
	app -> current_tool = t;

	if (app -> current_tool != T_MOVE) {
		if (app -> paste_executed) {
			for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
				for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
					if (app -> selection_grid[y][x] >= 0) {
						if (BETWEEN (y + (int)app -> move.offset.y, 0, GRID_TILE_COUNT_Y - 1) &&
							BETWEEN (x + (int)app -> move.offset.x, 0, GRID_TILE_COUNT_X - 1)) {
							app -> project.frames[app -> current_frame].grid[y + (int)app -> move.offset.y][x + (int)app -> move.offset.x] = 
								app -> selection_grid[y][x];
						}
					}
				}
			}
		}

		clear_selection (app);
		app -> paste_executed = false;
	}


	wnd_set_title (window, "Pixel Playground | %s |", text);
}

static void change_frame (pixel_app* app, change_frame_type type) {
	switch (type) {
		case CF_NEXT: {
			++app -> current_frame;
			if (!app -> is_playing) {
				if (app -> current_frame == app -> project.frame_count) {
					++app -> project.frame_count;
					for (unsigned y = 0; y < GRID_TILE_COUNT_X; ++y) {
						for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
							app -> project.frames[app -> current_frame].grid[y][x] = -1;
					}
				}
			}
			else {
				if (app -> current_frame == app -> project.frame_count)
					app -> current_frame = 0;
			}

			break;
		}
		case CF_PREV: {
			--app -> current_frame;
			break;
		}
		case CF_FIRST: {
			app -> current_frame = 0;
			break;
		}
		case CF_LAST: {
			app -> current_frame = app -> project.frame_count - 1;	
			break;
		}
		case CF_PLAY: {
			app -> current_frame = 0;
			app -> is_playing = !app -> is_playing;
			timer_reset (&app -> play_timer);
			break;
		}
	}

	app -> current_frame = CLAMP (app -> current_frame, 0, MAX_FRAME_COUNT - 1);
}

static void change_speed (pixel_app* app) {
	if (app -> speed == PS_FULL) {
		app -> speed = PS_HALF;
		app -> play_timer.target_miliseconds = 1000 / (FRAMES_PER_SECOND / 2);
	}
	else if (app -> speed == PS_HALF) {
		app -> speed = PS_FULL;
		app -> play_timer.target_miliseconds = 1000 / FRAMES_PER_SECOND;
	}
}

static void save (pixel_app* app) {
	char* path = NULL;
	if (io_show_save_file_dialog ("Save Pixel Playground Project", "Pixel Playground Project", "ppp", &path)) {
		file f = io_open (path, OM_WRITE);

		char* frame_count = str_format ("%d\n\n", app -> project.frame_count);
		str_buffer file_contents = make_str_buffer (frame_count);
		free (frame_count);

		for (unsigned i = 0; i < app -> project.frame_count; ++i) {
			unsigned start_index = 0;
			int previous_color = app -> project.frames[i].grid[0][0];
			for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
				for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
					int color = app -> project.frames[i].grid[y][x];
					if (color != previous_color) {
						unsigned end_index = x + y * GRID_TILE_COUNT_X;
						unsigned final_index = end_index - start_index;

						char* text = str_format ("%d[%d] ", previous_color, final_index);
						str_buffer_insert_text (&file_contents, text);
						free (text);

						start_index = end_index;
					}

					previous_color = color;
				}
			}

			char* text = str_format ("%d[%d] ", previous_color, (GRID_TILE_COUNT_Y * GRID_TILE_COUNT_X) - start_index);
			str_buffer_insert_text (&file_contents, text);
			free (text);

			str_buffer_insert_text (&file_contents, "-\n");
		}

		char* full_text = str_buffer_get_text (file_contents);
		io_write (&f, full_text);
		free (full_text);

		io_close (&f);
	}

	free (path);
}

static void load (pixel_app* app) {
	char* path = NULL;
	if (io_show_load_file_dialog ("Load Pixel Playground Project", "Pixel Playground Project", "ppp", &path)) {
	 	file f = io_open (path, OM_READ);

	 	if (io_read (&f))
	 		app -> project = parse_ppp (f.contents);

	 	io_close (&f);
	 }

	 free (path);
}

static bool draw_button (rect r, pixel_input input, gui_image icon, bool disabled) {
	bool result = false;
	v4 outline_color = make_color (OUTLINE_COLOR, 255);
	v4 actual_color = make_color (DEFAULT_BUTTON_COLOR, 255);
	unsigned outline = DEFAULT_OUTLINE;

	if (!disabled) {
		if (is_point_in_rect (r, input.mouse_pos)) {
			outline_color = make_color (BUTTON_HOVER_OUTLINE_COLOR, 255);
			outline = BUTTON_HOVER_OUTLINE;
			if (input.lmb_down)
				actual_color = make_color (BUTTON_ACTIVE_COLOR, 255);

			result = input.lmb_up;
		}
	}
	
	rect actual_rect = r;
	actual_rect.x += outline;
	actual_rect.y += outline;
	actual_rect.width -= outline * 2;
	actual_rect.height -= outline * 2;

	gl_draw_rect (r, outline_color, disabled);
	gl_draw_rect (actual_rect, actual_color, disabled);
	gl_draw_image (r, make_color (255, 255, 255, 255), icon, disabled);

	return result;
}

static void draw_selected_rect (rect r, v4 color) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	gl_draw_rect (r, make_color (DEFAULT_BUTTON_ICON_COLOR, 255), false);
	gl_draw_rect (selected_rect, color, false);
}

static bool draw_selectable_rect (rect r, v4 color, v2 mouse_pos, bool mb, bool is_selected, bool disabled) {
	if (!disabled) {
		if (is_selected) {
			draw_selected_rect (r, color);
			return false;
		}
		else if (is_point_in_rect (r, mouse_pos)) {
			draw_selected_rect (r, color);
			return mb;
		}
	}
	
	gl_draw_rect (r, color, false);
	return false;
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
		(int)ICO_LAST_FRAME,
		(int)ICO_PAUSE
	};

	rect r = { };
	for (unsigned i = 0; i < 5; ++i) {
		r = make_rect (start_pos, (float)widths[i], SMALL_BUTTON_HEIGHT);
		start_pos.x += widths[i] + INNER_MARGIN;

		unsigned icon = (i == 2 && app -> is_playing) ? icons[5] : icons[i];
		bool disabled = i != 2 && app -> is_playing;

		if (draw_button (r, input, app -> icons[icon], disabled))
			change_frame (app, (change_frame_type)i);
	}
}

static void draw_frame (pixel_app* app, pixel_input input) {
	if (app -> is_playing) {
		if (timer_get_value (app -> play_timer) >= app -> play_timer.target_miliseconds) {
			change_frame (app, CF_NEXT);
			timer_reset (&app -> play_timer);
		}
	}

	v2 start_pos = make_v2 (GRID_POSITION);

	rect outline_rect = make_rect (start_pos,
								   GRID_TILE_SIZE * GRID_TILE_COUNT_X + GRID_OUTLINE * 2, 
								   GRID_TILE_SIZE * GRID_TILE_COUNT_Y + GRID_OUTLINE * 2);
	gl_draw_rect (outline_rect, make_color (OUTLINE_COLOR, 255), false);

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
			int index = app -> project.frames[app -> current_frame].grid[y][x];
			if (index < 0)
				tile_color = tile_colors[(x + offset) % 2];
			else
				tile_color = colors[index];

			tile_rect.x = start_pos.x + x * GRID_TILE_SIZE;
			tile_rect.y = start_pos.y + y * GRID_TILE_SIZE;

			if (app -> current_tool == T_MOVE)
				input.mouse_pos = make_v2 (-1, -1);

			bool is_selected = app -> selection_grid[y][x] >= 0 && app -> current_tool != T_MOVE;

			if (draw_selectable_rect (tile_rect, tile_color, input.mouse_pos, input.lmb_down, is_selected, app -> is_playing)) {
				if (app -> current_tool == T_DRAW)
					app -> project.frames[app -> current_frame].grid[y][x] = app -> color_index;
				else if (app -> current_tool == T_ERASE) 
					app -> project.frames[app -> current_frame].grid[y][x] = -1;
				else if (app -> current_tool == T_SELECT) {
					app -> selection_grid[y][x] = app -> project.frames[app -> current_frame].grid[y][x] >= 0 ? 
						app -> project.frames[app -> current_frame].grid[y][x] : -1;
					app -> tiles_selected = true;
				}
			}
		}
	}
}

static void handle_move (pixel_app* app, pixel_input input) {
	if (input.lmb_down && !app -> move.in_progress) {
		v2 start_pos = make_v2 (GRID_POSITION);
		rect outline_rect = make_rect (start_pos,
								   	   GRID_TILE_SIZE * GRID_TILE_COUNT_X + GRID_OUTLINE * 2, 
								       GRID_TILE_SIZE * GRID_TILE_COUNT_Y + GRID_OUTLINE * 2);
		if (is_point_in_rect (outline_rect, input.mouse_pos)) {
			app -> move.origin = input.mouse_pos;
			app -> move.in_progress = true;

			if (!app -> paste_executed) {
				for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
					for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
						if (app -> selection_grid[y][x] >= 0)
							app -> project.frames[app -> current_frame].grid[y][x] = -1;
					}
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

		if (!app -> paste_executed) {
			for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
				for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
					if (app -> selection_grid[y][x] >= 0) {
						if (BETWEEN (y + (int)app -> move.offset.y, 0, GRID_TILE_COUNT_Y - 1) &&
							BETWEEN (x + (int)app -> move.offset.x, 0, GRID_TILE_COUNT_X - 1)) {
							app -> project.frames[app -> current_frame].grid[y + (int)app -> move.offset.y][x + (int)app -> move.offset.x] = 
								app -> selection_grid[y][x];
						}
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
}

static void draw_selected_pixels (pixel_app* app, pixel_input input) {
	if (app -> current_tool == T_MOVE)
		handle_move (app, input);
	
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
				gl_draw_rect (tile_rect, tile_color, false);
		}
	}
}

static void draw_colors (pixel_app* app, pixel_input input) {
	v2 start_pos = make_v2 (COLORS_POSITION);

	rect outline_rect = make_rect (start_pos, 
								   COLOR_TILE_SIZE * COLOR_TILE_COUNT_X + GRID_OUTLINE * 2,
								   COLOR_TILE_SIZE * COLOR_TILE_COUNT_Y + GRID_OUTLINE * 2);
	gl_draw_rect (outline_rect, make_color (OUTLINE_COLOR, 255), false);

	start_pos.x += GRID_OUTLINE;
	start_pos.y += GRID_OUTLINE;

	rect tile_rect = make_rect (start_pos, COLOR_TILE_SIZE, COLOR_TILE_SIZE);

	v4 tile_color = colors[0];
	for (unsigned y = 0; y < COLOR_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < COLOR_TILE_COUNT_X; ++x) {
			tile_color = colors[x + y * COLOR_TILE_COUNT_X];

			tile_rect.x = start_pos.x + x * COLOR_TILE_SIZE;
			tile_rect.y = start_pos.y + y * COLOR_TILE_SIZE;

			if (draw_selectable_rect (tile_rect, tile_color, input.mouse_pos, input.lmb_up, 
				app -> color_index == x + y * COLOR_TILE_COUNT_X, app -> is_playing)) {
				app -> color_index = x + y * COLOR_TILE_COUNT_X;
			}
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

	if (draw_button (rects[0], input, app -> icons[(int)ICO_DRAW], app -> is_playing))
		set_tool (app, T_DRAW, "Draw Tool", window);
	if (draw_button (rects[1], input, app -> icons[(int)ICO_ERASE], app -> is_playing))
		set_tool (app, T_ERASE, "Erase Tool", window);
	if (draw_button (rects[2], input, app -> icons[(int)ICO_SELECT], app -> is_playing))
		set_tool (app, T_SELECT, "Select Tool", window);
	if (draw_button (rects[3], input, app -> icons[(int)ICO_MOVE], app -> is_playing))
		set_tool (app, T_MOVE, "Move Tool", window);
	if (draw_button (rects[4], input, app -> icons[(int)ICO_COPY], app -> is_playing)) {
		copy_to_clipboard (app);
	}
	if (draw_button (rects[5], input, app -> icons[(int)ICO_PASTE], app -> is_playing))
		paste (app);
	if (draw_button (rects[6], input, app -> icons[app -> speed == PS_FULL ? (int)ICO_FULL_SPEED : (int)ICO_HALF_SPEED], app -> is_playing))
		change_speed (app);
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

	if (draw_button (clear_rect, input, app -> icons[(int)ICO_CLEAR], app -> is_playing)) {
		for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
			for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
				app -> project.frames[app -> current_frame].grid[y][x] = -1;
		}
	}
	if (draw_button (save_rect, input, app -> icons[(int)ICO_SAVE], app -> is_playing))
		save (app);
	if (draw_button (load_rect, input, app -> icons[(int)ICO_LOAD], app -> is_playing))
		load (app);
	if (draw_button (export_rect, input, app -> icons[(int)ICO_EXPORT], app -> is_playing))
		io_log ("Export animation");
}

void pixel_init (gui_window window, void* memory) {
	pixel_app* app = (pixel_app*)memory;
	app -> color_index = 0;

	app -> current_frame = 0;
	app -> project.frame_count = 1;
	for (unsigned y = 0; y < GRID_TILE_COUNT_Y; ++y) {
		for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x)
			app -> project.frames[app -> current_frame].grid[y][x] = -1;
	}

	for (unsigned i = 0; i < ICON_COUNT; ++i) {
		resources_load_image (icons[i], &app -> icons[(Icon)i]);
		gl_load_image (&app -> icons[(Icon)i]);
	}

	set_tool (app, T_DRAW, "Draw Tool", window);

	app -> move.in_progress = false;
	app -> move.offset = make_v2 (0, 0);

	app -> tiles_selected = false;
	app -> is_playing = false;

	app -> play_timer = { };
	app -> play_timer.target_miliseconds = 1000 / FRAMES_PER_SECOND;
	app -> speed = PS_FULL;

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
	draw_selected_pixels (app, input);

	gl_end_clip_rect ();

	draw_colors (app, input);
	draw_tools (app, input, window);
	draw_buttons (app, input);
}