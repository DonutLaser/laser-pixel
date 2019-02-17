#include "pixel.h"

#include "pixel_gl.h"
#include "pixel_parser.h"

#include "../third_party/gui_io.h"
#include "../third_party/gui_window.h"
#include "../third_party/gui_string_buffer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "../third_party/stb_image_write.h"

struct selection_neighbor_info {
	bool sides[4];
	bool corners[4];
};

enum Icon { ICO_FIRST_FRAME, ICO_PREV_FRAME, ICO_PLAY, ICO_PAUSE, ICO_NEXT_FRAME, ICO_LAST_FRAME,
			ICO_DRAW, ICO_ERASE, ICO_SELECT, ICO_MOVE, ICO_COPY, ICO_PASTE, ICO_CLEAR,
			ICO_SAVE, ICO_LOAD, ICO_EXPORT, ICO_FULL_SPEED, ICO_HALF_SPEED };

enum cursor { CUR_DRAW, CUR_ERASE, CUR_SELECT, CUR_MOVE };

enum change_frame_type { CF_FIRST, CF_PREV, CF_PLAY, CF_NEXT, CF_LAST };

static const char* tool_to_string (tool t) {
	switch (t) {
		case T_DRAW:
			return "Draw";
		case T_ERASE:
			return "Erase";
		case T_SELECT:
			return "Select";
		case T_MOVE:
			return "Move";
	}

	return NULL;
}

static void update_title (pixel_app* app, gui_window window) {
	wnd_set_title (window, "Pixel Playground - %s tool - Frame: %d/%d", 
				   tool_to_string (app -> current_tool), app -> current_frame + 1, app -> project.frame_count);
}

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

	update_title (app, window);
}

static void change_frame (pixel_app* app, change_frame_type type, gui_window window) {
	switch (type) {
		case CF_NEXT: {
			if (app -> project.frame_count == MAX_FRAME_COUNT)
				break;

			++app -> current_frame;
			if (!app -> is_playing) {
				if (app -> current_frame == app -> project.frame_count) {
					++app -> project.frame_count;
					for (unsigned y = 0; y < GRID_TILE_COUNT_X; ++y) {
						for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
							app -> project.frames[app -> current_frame].grid[y][x] = 
								app -> project.frames[app -> current_frame - 1].grid[y][x];
						}
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

	update_title (app, window);
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

		free (path);
	}

}

static void load (pixel_app* app) {
	char* path = NULL;
	if (io_show_load_file_dialog ("Load Pixel Playground Project", "Pixel Playground Project", "ppp", &path)) {
	 	file f = io_open (path, OM_READ);

	 	if (io_read (&f))
	 		app -> project = parse_ppp (f.contents);

	 	io_close (&f);

		free (path);
	 }
}

static void export_to_images (pixel_app* app) {
	char* path = NULL;
	if (io_show_select_folder_dialog ("Select the folder to save images to", &path)) {
		for (unsigned i = 0; i < app -> project.frame_count; ++i) {
			char data[GRID_TILE_COUNT_Y * 4 * 4 * GRID_TILE_COUNT_X * 4 * 4];
			unsigned index = 0;
			for (unsigned y = 0; y < GRID_TILE_COUNT_Y * 4; ++y) {
				for (unsigned x = 0; x < GRID_TILE_COUNT_X; ++x) {
					int color = app -> project.frames[i].grid[y / 4][x];
					v4 color_value = color < 0 ? make_color (0, 0, 0, 0) : colors[color];
					for (unsigned c = 0; c < 4 * 4; ++c) { 
						data[index] = (char)(color_value.f[c % 4] * 255.0f);
						++index;
					}
				}
			}

			char* file_name = str_format ("\\Frame%d.png", i);
			char* full_path = str_concatenate (path, file_name);
			stbi_write_png (full_path, GRID_TILE_COUNT_Y * 4, GRID_TILE_COUNT_X * 4, 4, (void*)data, GRID_TILE_COUNT_Y * 4 * 4);
			free (file_name);
			free (full_path);
		}

		free (path);
	}
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

static void draw_selected_rect (rect r, v4 color, selection_neighbor_info info) {
	rect selected_rect = r;
	selected_rect.x += SELECTION_INDICATOR_OUTLINE;
	selected_rect.y += SELECTION_INDICATOR_OUTLINE;
	selected_rect.width -= SELECTION_INDICATOR_OUTLINE * 2;
	selected_rect.height -= SELECTION_INDICATOR_OUTLINE * 2;

	enum sides { S_TOP, S_RIGHT, S_BOTTOM, S_LEFT };

	if (info.sides[S_TOP]) {
		selected_rect.y -= SELECTION_INDICATOR_OUTLINE;
		selected_rect.height += SELECTION_INDICATOR_OUTLINE;
	}

	if (info.sides[S_RIGHT])
		selected_rect.width += SELECTION_INDICATOR_OUTLINE;

	if (info.sides[S_BOTTOM])
		selected_rect.height += SELECTION_INDICATOR_OUTLINE;

	if (info.sides[S_LEFT]) {
		selected_rect.x -= SELECTION_INDICATOR_OUTLINE;
		selected_rect.width += SELECTION_INDICATOR_OUTLINE;
	}

	v4 outline_color = make_color (DEFAULT_BUTTON_ICON_COLOR, 255);

	gl_draw_rect (r, outline_color, false);
	gl_draw_rect (selected_rect, color, false);

	rect corner = { };
	corner.width = corner.height = SELECTION_INDICATOR_OUTLINE;

	enum corners { C_TOP_LEFT, C_TOP_RIGHT, C_BOTTOM_LEFT, C_BOTTOM_RIGHT };

	if (info.sides[S_TOP] && info.sides[S_LEFT] && !info.corners[C_TOP_LEFT]) {
		corner.x = r.x;
		corner.y = r.y;
		gl_draw_rect (corner, outline_color, false);
	}

	if (info.sides[S_TOP] && info.sides[S_RIGHT] && !info.corners[C_TOP_RIGHT]) {
		corner.x = r.x + (r.width - SELECTION_INDICATOR_OUTLINE); 
		corner.y = r.y;
		gl_draw_rect (corner, outline_color, false);
	}

	if (info.sides[S_BOTTOM] && info.sides[S_LEFT] && !info.corners[C_BOTTOM_LEFT]) {
		corner.x = r.x;
		corner.y = r.y + (r.height - SELECTION_INDICATOR_OUTLINE);
		gl_draw_rect (corner, outline_color, false);
	}

	if (info.sides[S_RIGHT] && info.sides[S_BOTTOM] && !info.corners[C_BOTTOM_RIGHT]) {
		corner.x = r.x + (r.width - SELECTION_INDICATOR_OUTLINE);
		corner.y = r.y + (r.height - SELECTION_INDICATOR_OUTLINE);
		gl_draw_rect (corner, outline_color, false);
	}
}

static bool draw_selectable_rect (rect r, v4 color, v2 mouse_pos, bool mb, bool is_selected, selection_neighbor_info info, bool disabled) {
	if (!disabled) {
		if (is_selected) {
			draw_selected_rect (r, color, info);
			return false;
		}
		else if (is_point_in_rect (r, mouse_pos)) {
			draw_selected_rect (r, color, info);
			return mb;
		}
	}
	
	gl_draw_rect (r, color, false);
	return false;
}
 
static void fill_color (pixel_app* app, int x, int y) {
	int color_to_change = app -> project.frames[app -> current_frame].grid[y][x];
	app -> project.frames[app -> current_frame].grid[y][x] = app -> color_index;

	struct coordinates {
		int x;
		int y;
	};

	coordinates coords[GRID_TILE_COUNT_X * GRID_TILE_COUNT_Y];
	int current_coord = 0;

	int new_x = x;
	int new_y = y;
	while (current_coord >= 0) {
		// For some reason, the loop doesn't terminate when calling this function without a breakpoint,
		// but it does terminate when a breakpoint is present. current_coord changes from negative to positive
		// value before the condition check. Some kind of a memory corruption? 
		// Also, the color_to_change changes seemingly
		// on its own, no clue why. This prevents the loop from going on infinitely.
		if (color_to_change == app -> color_index)
			break;

		new_x += 1;
		if (new_x < GRID_TILE_COUNT_X) {
			if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_change) {
				app -> project.frames[app -> current_frame].grid[new_y][new_x] = app -> color_index;
				coords[current_coord++] = { new_x, new_y };
				continue;
			}
		}

		new_x -= 1;
		new_y += 1;
		if (new_y < GRID_TILE_COUNT_Y) {
			if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_change) {
		 		app -> project.frames[app -> current_frame].grid[new_y][new_x] = app -> color_index;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		new_x -= 1;
		new_y -= 1;
		if (new_x >= 0) {
		 	if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_change) {
		 		app -> project.frames[app -> current_frame].grid[new_y][new_x] = app -> color_index;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		new_x += 1;
		new_y -= 1;
		if (new_y >= 0) {
		 	if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_change) {
		 		app -> project.frames[app -> current_frame].grid[new_y][new_x] = app -> color_index;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		if (current_coord != 0) {
			coordinates last_coords = coords[--current_coord];
			new_x = last_coords.x;
			new_y = last_coords.y;
		}
		else
			break;
	}
}

static void fill_line (pixel_app* app, int x, int y) {
	int x_diff = x - app -> last_x; 
	int y_diff = y - app -> last_y;

	if (ABS (x_diff) >= ABS (y_diff)) {
		int dir = x_diff < 0 ? -1 : 1;

		while (app -> last_x != x) {
			app -> last_x += dir;
			app -> project.frames[app -> current_frame].grid[app -> last_y][app -> last_x] = app -> color_index;
		}
	}
	else {
		int dir = y_diff < 0 ? -1 : 1;

		while (app -> last_y != y) {
			app -> last_y += dir;
			app -> project.frames[app -> current_frame].grid[app -> last_y][app -> last_x] = app -> color_index;
		}
	}
}

static void select_area (pixel_app* app, unsigned x, unsigned y) {
	int color_to_select = app -> project.frames[app -> current_frame].grid[y][x];
	app -> selection_grid[y][x] = color_to_select;

	struct coordinates {
		int x;
		int y;
	};

	coordinates coords[GRID_TILE_COUNT_X * GRID_TILE_COUNT_Y];
	int current_coord = 0;

	int new_x = x;
	int new_y = y;
	while (current_coord >= 0) {
		new_x += 1;
		if (new_x < GRID_TILE_COUNT_X) {
			if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_select &&
				app -> selection_grid[new_y][new_x] < 0) {
				app -> selection_grid[new_y][new_x] = color_to_select;
				coords[current_coord++] = { new_x, new_y };
				continue;
			}
		}

		new_x -= 1;
		new_y += 1;
		if (new_y < GRID_TILE_COUNT_Y) {
			if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_select &&
				app -> selection_grid[new_y][new_x] < 0) {
				app -> selection_grid[new_y][new_x] = color_to_select;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		new_x -= 1;
		new_y -= 1;
		if (new_x >= 0) {
		 	if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_select &&
		 		app -> selection_grid[new_y][new_x] < 0) {
				app -> selection_grid[new_y][new_x] = color_to_select;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		new_x += 1;
		new_y -= 1;
		if (new_y >= 0) {
		 	if (app -> project.frames[app -> current_frame].grid[new_y][new_x] == color_to_select &&
		 		app -> selection_grid[new_y][new_x] < 0) {
				app -> selection_grid[new_y][new_x] = color_to_select;
		 		coords[current_coord++] = { new_x, new_y };
		 		continue;
		 	}
		}

		if (current_coord != 0) {
			coordinates last_coords = coords[--current_coord];
			new_x = last_coords.x;
			new_y = last_coords.y;
		}
		else
			break;
	}
}

static void select_line (pixel_app* app, int x, int y) {
	int x_diff = x - app -> last_x; 
	int y_diff = y - app -> last_y;

	if (ABS (x_diff) >= ABS (y_diff)) {
		int dir = x_diff < 0 ? -1 : 1;

		while (app -> last_x != x) {
			app -> last_x += dir;
			app -> selection_grid[app -> last_y][app -> last_x] = app -> project.frames[app -> current_frame].grid[app -> last_y][app -> last_x];
		}
	}
	else {
		int dir = y_diff < 0 ? -1 : 1;

		while (app -> last_y != y) {
			app -> last_y += dir;
			app -> selection_grid[app -> last_y][app -> last_x] = app -> project.frames[app -> current_frame].grid[app -> last_y][app -> last_x];
		}
	}
}

static selection_neighbor_info get_selection_info (pixel_app* app, unsigned x, unsigned y) {
	selection_neighbor_info result = { };

	int new_x = x;
	int new_y = y;

	struct offset {
		int x;
		int y;
	};

	offset offsets[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	for (unsigned i = 0; i < 4; ++i) {
		new_y += offsets[i].y;
		new_x += offsets[i].x;

		result.sides[i] = BETWEEN (new_y, 0, GRID_TILE_COUNT_Y - 1) &&
			BETWEEN (new_x, 0, GRID_TILE_COUNT_X - 1) && (app -> selection_grid[new_y][new_x] >= 0);

		new_y = y;
		new_x = x;
	}

	offset corner_offsets[4] = { { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };
	for (unsigned i = 0; i < 4; ++i) {
		new_y += corner_offsets[i].y;
		new_x += corner_offsets[i].x;

		result.corners[i] = BETWEEN (new_y, 0, GRID_TILE_COUNT_Y - 1) &&
			BETWEEN (new_x, 0, GRID_TILE_COUNT_X - 1) && (app -> selection_grid[new_y][new_x] >= 0);

		new_y = y;
		new_x = x;
	}

	return result;
}

static void draw_controls (pixel_app* app, pixel_input input, gui_window window) {
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
			change_frame (app, (change_frame_type)i, window);
	}
}

static void draw_frame (pixel_app* app, pixel_input input, gui_window window) {
	if (app -> is_playing) {
		if (timer_get_value (app -> play_timer) >= app -> play_timer.target_miliseconds) {
			change_frame (app, CF_NEXT, window);
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
			selection_neighbor_info selection_info = { };
			if (is_selected)
				selection_info = get_selection_info (app, y, x);

			if (draw_selectable_rect (tile_rect, tile_color, input.mouse_pos, input.lmb_down, is_selected, selection_info, app -> is_playing)) {
				if (app -> current_tool == T_DRAW) {
					if (input.ctrl_pressed)
						fill_color (app, x, y);
					else if (input.shift_pressed) {
						if (app -> last_x >= 0 || app -> last_y >= 0)
							fill_line (app, x, y);
					}
					else {
						app -> project.frames[app -> current_frame].grid[y][x] = app -> color_index;
						app -> last_x = x;
						app -> last_y = y;
					}
				}
				else if (app -> current_tool == T_ERASE) 
					app -> project.frames[app -> current_frame].grid[y][x] = -1;
				else if (app -> current_tool == T_SELECT) {
					if (input.ctrl_pressed)
						select_area (app, x, y);
					else if (input.shift_pressed) {
						if (app -> last_x >= 0 || app -> last_y >= 0)
							select_line (app, x, y);
					}
					else {
						app -> selection_grid[y][x] = app -> project.frames[app -> current_frame].grid[y][x] >= 0 ? 
							app -> project.frames[app -> current_frame].grid[y][x] : -1;
						app -> last_x = x;
						app -> last_y = y;
					}
					
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

			selection_neighbor_info selection_info = get_selection_info (app, x, y);

 			if (app -> selection_grid[y][x] >= 0) 
 				draw_selected_rect (tile_rect, tile_color, selection_info);
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
				app -> color_index == x + y * COLOR_TILE_COUNT_X, { }, app -> is_playing)) {
				app -> color_index = x + y * COLOR_TILE_COUNT_X;
			}
		}
	}
}

static void draw_tools (pixel_app* app, pixel_input input, gui_window* window) {
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
		set_tool (app, T_DRAW, "Draw Tool", *window);
	if (draw_button (rects[1], input, app -> icons[(int)ICO_ERASE], app -> is_playing))
		set_tool (app, T_ERASE, "Erase Tool", *window);
	if (draw_button (rects[2], input, app -> icons[(int)ICO_SELECT], app -> is_playing))
		set_tool (app, T_SELECT, "Select Tool", *window);
	if (draw_button (rects[3], input, app -> icons[(int)ICO_MOVE], app -> is_playing))
		set_tool (app, T_MOVE, "Move Tool", *window);
	if (draw_button (rects[4], input, app -> icons[(int)ICO_COPY], app -> is_playing))
		copy_to_clipboard (app);
	if (draw_button (rects[5], input, app -> icons[(int)ICO_PASTE], app -> is_playing))
		paste (app);
	if (draw_button (rects[6], input, app -> icons[app -> speed == PS_FULL ? (int)ICO_FULL_SPEED : (int)ICO_HALF_SPEED], false))
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

		app -> last_x = app -> last_y = -1;
	}
	if (draw_button (save_rect, input, app -> icons[(int)ICO_SAVE], app -> is_playing))
		save (app);
	if (draw_button (load_rect, input, app -> icons[(int)ICO_LOAD], app -> is_playing))
		load (app);
	if (draw_button (export_rect, input, app -> icons[(int)ICO_EXPORT], app -> is_playing))
		export_to_images (app);
}

void pixel_init (gui_window* window, void* memory) {
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

	set_tool (app, T_DRAW, "Draw Tool", *window);
	app -> last_x = app -> last_y = -1;

	app -> move.in_progress = false;
	app -> move.offset = make_v2 (0, 0);

	app -> tiles_selected = false;
	app -> is_playing = false;

	app -> play_timer = { };
	app -> play_timer.target_miliseconds = 1000 / FRAMES_PER_SECOND;
	app -> speed = PS_FULL;

	gl_init (*window);

	for (unsigned i = 0; i < CURSOR_COUNT; ++i)
		app -> cursors[i] = wnd_generate_new_cursor (cursors[i]);
}

void pixel_update (void* memory, pixel_input input, gui_window* window) {
	pixel_app* app = (pixel_app*)memory;

	draw_controls (app, input, *window);

	rect clip_rect = make_rect (make_v2 (GRID_POSITION),
								GRID_TILE_SIZE * GRID_TILE_COUNT_X,
								GRID_TILE_SIZE * GRID_TILE_COUNT_Y);
	clip_rect.x += GRID_OUTLINE;
	clip_rect.y += GRID_OUTLINE;
	gl_begin_clip_rect (wnd_get_client_size (*window), clip_rect);

	if (is_point_in_rect (clip_rect, input.mouse_pos) && !app -> custom_cursor) {
		switch (app -> current_tool) {
			case T_DRAW: {
				wnd_set_cursor (window, app -> cursors[CUR_DRAW]);
				break;
			}
			case T_ERASE: {
				wnd_set_cursor (window, app -> cursors[CUR_ERASE]);
				break;
			}
			case T_SELECT: {
				wnd_set_cursor (window, app -> cursors[CUR_SELECT]);
				break;
			}
			case T_MOVE: {
				wnd_set_cursor (window, app -> cursors[CUR_MOVE]);
				break;
			}
		}

		app -> custom_cursor = true;
	}
	else if (!is_point_in_rect (clip_rect, input.mouse_pos) && app -> custom_cursor) {
		wnd_set_cursor (window, NULL);
		app -> custom_cursor = false;
	}

	draw_frame (app, input, *window);
	draw_selected_pixels (app, input);

	gl_end_clip_rect ();

	draw_colors (app, input);
	draw_tools (app, input, window);
	draw_buttons (app, input);
}