#if !defined (GUI_RESOURCES_H)
#define GUI_RESOURCES_H

#include "../third_party/gui_math.h"

struct gui_image {
	void* data;
	v2 size;

	unsigned id;
};

struct font_character {
	gui_image bitmap;
	v2 offset;
	int advance;
};

struct gui_font {
	font_character chars[128];
	void* freetype_face;
	bool has_kerning;
};

bool resources_load_image (const char* path, gui_image* result);

#endif