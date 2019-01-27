#if !defined (CONSTANTS_H)
#define CONSTANTS_H

#include "../third_party/gui_math.h"

#define OUTER_MARGIN 				10
#define INNER_MARGIN 				5
#define GRID_TILE_SIZE 				16
#define COLOR_TILE_SIZE 			26
#define GRID_OUTLINE 				2
#define DEFAULT_OUTLINE				1
#define SMALL_BUTTON_HEIGHT 		24
#define LARGE_BUTTON_HEIGHT 		45
#define FRAME_BUTTON_WIDTH			40
#define PLAY_BUTTON_WIDTH 			64
#define LARGE_BUTTON_WIDTH 			30
#define SPEED_WIDTH 				64
#define CLEAR_BUTTON_WIDTH			84
#define SAVE_BUTTON_WIDTH			86
#define LOAD_BUTTON_WIDTH			86
#define EXPORT_BUTTON_WIDTH 		128
#define BUTTON_HOVER_OUTLINE		2
#define SELECTION_INDICATOR_OUTLINE 2

#define GRID_TILE_COUNT_X			16
#define GRID_TILE_COUNT_Y			16
#define COLOR_TILE_COUNT_X			5
#define COLOR_TILE_COUNT_Y			6

static v3 colors[] = {
	{ 128, 34, 34 }, { 239, 23, 23 }, { 239, 114, 23 }, { 239, 216, 23 }, { 246, 255, 2 },
	{ 32, 52, 128 }, { 23, 43, 239 }, { 23, 135, 239 }, { 23, 206, 239 }, { 2, 255, 240 },
	{ 49, 128, 34 }, { 69, 239, 23 }, { 125, 239, 23 }, { 175, 239, 23 }, { 198, 255, 2 },
	{ 120, 32, 128 }, { 216, 23, 239 }, { 239, 23, 216 }, { 239, 23, 145 }, { 255, 2, 139 },
	{ 60, 60, 60 }, { 107, 106, 106 }, { 147, 146, 146 }, { 185, 185, 185 }, { 225, 224, 224 },
	{ 255, 204, 166 }, { 60, 175, 131 }, { 110, 81, 46 }, { 242, 139, 160 }, { 255, 255, 255 }
};

#define BACKGROUND_COLOR			41, 49, 57
#define OUTLINE_COLOR				10, 22, 31

#define GRID_TILE_LIGHT				207, 207, 207
#define GRID_TILE_DARK				174, 171, 171

#define DEFAULT_BUTTON_COLOR		45, 60, 73
#define DEFAULT_BUTTON_ICON_COLOR	245, 234, 159
#define PLAY_BUTTON_ICON_COLOR		240, 63, 63
#define SPEED_COLOR					234, 234, 234
#define CLEAR_BUTTON_TEXT_COLOR		245, 159, 159
#define SAVE_BUTTON_TEXT_COLOR		165, 245, 159
#define LOAD_BUTTON_TEXT_COLOR		245, 208, 159
#define EXPORT_BUTTON_TEXT_COLOR	159, 200, 245
#define BUTTON_HOVER_OUTLINE_COLOR	41, 105, 158
#define BUTTON_ACTIVE_COLOR			26, 78, 121

#define BUTTON_TEXT_SIZE			16
#define SPEED_TEXT_SIZE				22

#define SHADOW_OFFSET_X 			0
#define SHADOW_OFFSET_Y				2
#define SHADOW_COLOR				13, 18, 22

#define CONTROLS_OFFSET				80

#endif