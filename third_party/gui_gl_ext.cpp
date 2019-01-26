#include "gui_gl_ext.h"

#include "gui_window.h"
#include "gui_io.h"

void* load_method (const char* name) {
	void* p = (void*)wglGetProcAddress (name);

	if (!p) {
		HMODULE module = LoadLibraryA ("opengl32.dll");
		p = (void*)GetProcAddress (module, name);

		if (!p)
			io_log_error ("GL method %s could not be loaded", name);
	}

	return p;
}
