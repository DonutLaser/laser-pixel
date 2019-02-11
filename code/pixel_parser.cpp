#include "pixel_parser.h"

#include "../third_party/gui_string_buffer.h"
#include "../third_party/gui_utility.h"

#define SYMBOL_LEFT_BRACKET			'['
#define SYMBOL_RIGHT_BRACKET		']'
#define SYMBOL_FRAME_END			'-'

static bool is_whitespace (char c) {
	return c == '\n' || c == '\t' || c == '\r' || c == ' ';
}

static void skip_whitespace (char** text) {
	while (is_whitespace (**text))
		++(*text);
}

static token get_next_token (char** text) {
	skip_whitespace (text);

	bool is_color_value = false;

	unsigned count = 0;
	token result = { };
	result.type = TT_COUNT;
	char* value_start = *text;
	while (!is_whitespace (**text)) {
		if (**text == SYMBOL_LEFT_BRACKET || **text == SYMBOL_RIGHT_BRACKET)
			result.type = TT_COLOR;
		else if (**text == SYMBOL_FRAME_END)
			result.type = TT_FRAME_END;

		++count;
		++(*text);
	}

	str_copy (value_start, result.value, 0, count);
	result.value[count] = '\0';

	return result;
}

static int parse_number (char* value) {
	return utility_is_integer (value) ? utility_string_to_int (value) : 0;
} 

static void parse_color (char* value, int* color, unsigned* amount) {
	unsigned count = 0;
	char* value_start = value;
	while (*value != '\0') {
		if (*value == SYMBOL_LEFT_BRACKET) {
			char color_value[MAX_TOKEN_LENGTH];
			str_copy (value_start, color_value, 0, count);
			color_value[count] = '\0';
			*color = parse_number (color_value);

			count = 0;
			++value;
			value_start = value;
			continue;
		}
		else if (*value == SYMBOL_RIGHT_BRACKET) {
			char amount_value[MAX_TOKEN_LENGTH];
			str_copy (value_start, amount_value, 0, count);
			amount_value[count] = '\0';
			*amount = (unsigned)parse_number (amount_value);
		}

		++count;
		++value;
	}
}

ppp parse_ppp (char* text) {
	ppp result = { };

	unsigned start_x = 0;
	unsigned start_y = 0;
	unsigned frame = 0;
	token t = { };
	do {
		t = get_next_token (&text);

		unsigned amount = 0;
		int color = -1;
		if (t.type == TT_COUNT)
			result.frame_count = (unsigned)parse_number (t.value);
		else if (t.type == TT_COLOR) {
			if (amount == 0) {
				color = -1;
				amount = 0;
				parse_color (t.value, &color, &amount);
			}

			for (unsigned y = start_y; y < GRID_TILE_COUNT_Y; ++y) {
				for (unsigned x = start_x; x < GRID_TILE_COUNT_X; ++x) {
					result.frames[frame].grid[y][x] = color;
					--amount;

					if (amount == 0) {
						if (x < GRID_TILE_COUNT_X - 1) {
							start_x = x + 1;
							start_y = y;
						}
						else {
							start_x = 0;
							start_y = y + 1;
						}

						break;
					}
				}

				if (amount == 0)
					break;
				else
					start_x = 0;
			}
		}
		else if (t.type == TT_FRAME_END) {
			++frame;
			start_x = 0;
			start_y = 0;
		}
	}
	while (frame != result.frame_count);

	return result;
}