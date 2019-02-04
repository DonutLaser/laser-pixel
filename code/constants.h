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

static v4 colors[] = {
	make_color (128, 34, 34, 255), 
	make_color (239, 23, 23, 255), 
	make_color (239, 114, 23, 255), 
	make_color (239, 216, 23, 255), 
	make_color (246, 255, 2, 255),
	make_color (32, 52, 128, 255), 
	make_color (23, 43, 239, 255), 
	make_color (23, 135, 239, 255), 
	make_color (23, 206, 239, 255), 
	make_color (2, 255, 240, 255),
	make_color (49, 128, 34, 255), 
	make_color (69, 239, 23, 255), 
	make_color (125, 239, 23, 255), 
	make_color (175, 239, 23, 255), 
	make_color (198, 255, 2, 255),
	make_color (120, 32, 128, 255), 
	make_color (216, 23, 239, 255), 
	make_color (239, 23, 216, 255), 
	make_color (239, 23, 145, 255),
	make_color (255, 2, 139, 255),
	make_color (60, 60, 60, 255), 
	make_color (107, 106, 106, 255), 
	make_color (147, 146, 146, 255), 
	make_color (185, 185, 185, 255), 
	make_color (225, 224, 224, 255),
	make_color (255, 204, 166, 255), 
	make_color (60, 175, 131, 255), 
	make_color (110, 81, 46, 255), 
	make_color (242, 139, 160, 255), 
	make_color (255, 255, 255, 255)
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

#define CONTROLS_POSITION			80, 10
#define GRID_POSITION				10, 39
#define COLORS_POSITION				275, 39
#define TOOLS_POSITION 				275, 204
#define BUTTONS_POSITION 			10, 304

#define ICON_COUNT 					18
static const char* icons[ICON_COUNT] = {
	"W://pixel//data//images//first_frame.png",
	"W://pixel//data//images//prev_frame.png",
	"W://pixel//data//images//play.png",
	"W://pixel//data//images//pause.png",
	"W://pixel//data//images//next_frame.png",
	"W://pixel//data//images//last_frame.png",
	"W://pixel//data//images//draw.png",
	"W://pixel//data//images//erase.png",
	"W://pixel//data//images//select.png",
	"W://pixel//data//images//move.png",
	"W://pixel//data//images//copy.png",
	"W://pixel//data//images//paste.png",
	"W://pixel//data//images//clear.png",
	"W://pixel//data//images//save.png",
	"W://pixel//data//images//load.png",
	"W://pixel//data//images//export.png",
	"W://pixel//data//images//full_speed.png",
	"W://pixel//data//images//half_speed.png"
};

#define MAX_FRAME_COUNT				512	

#define FRAMES_PER_SECOND			6

#endif