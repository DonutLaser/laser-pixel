#if !defined (PIXEL_PARSER_H)
#define PIXEL_PARSER_H

#include "constants.h"

#define MAX_TOKEN_LENGTH	10	

enum token_type { TT_COLOR, TT_FRAME_END, TT_COUNT, TT_EOF };

struct frame {
	int grid[GRID_TILE_COUNT_Y][GRID_TILE_COUNT_X];
};

struct ppp {
	unsigned frame_count;
	frame frames[MAX_FRAME_COUNT];
};

struct token {
	token_type type;
	char value[MAX_TOKEN_LENGTH];
};

ppp parse_ppp (char* text);

#endif