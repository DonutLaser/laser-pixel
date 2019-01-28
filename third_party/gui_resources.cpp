#include "gui_resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

bool resources_load_image (const char* path, gui_image* result) {
	*result = { };
	int w, h, n;
	result -> data = stbi_load (path, &w, &h, &n, 0);

	if (result -> data != NULL)
		result -> size = make_v2 (w, h);

	return result -> data != NULL;
}