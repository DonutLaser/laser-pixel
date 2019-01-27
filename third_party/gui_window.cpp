#include "gui_window.h"

#include <gl/gl.h>
#include <stdio.h>

#include "gui_io.h"
#include "gui_utility.h"

// WGL extensions
#define WGL_DRAW_TO_WINDOW_ARB				0x2001
#define WGL_SUPPORT_OPENGL_ARB				0x2010
#define WGL_DOUBLE_BUFFER_ARB				0x2011
#define WGL_PIXEL_TYPE_ARB					0x2013
#define WGL_TYPE_RGBA_ARB					0x202B
#define WGL_COLOR_BITS_ARB					0x2014
#define WGL_DEPTH_BITS_ARB					0x2022
#define WGL_STENCIL_BITS_ARB				0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB		0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB		0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB		0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB	0x00000001

typedef bool (*WGL_CHOOSEPIXELFORMATARB) (HDC dc, const int* attrib_list, const float* f_attrib_list, unsigned max_formats, int* formats, unsigned* formats_count); // wglChoosePixelFormatARB
typedef HGLRC (*WGL_CREATECONTEXTATTRIBSARB) (HDC dc, HGLRC share_ctx, const int* attrib_list); // wglCreateContextAttribsARB ()

enum input_modifier { IM_CTRL = 0x01, IM_ALT = 0x02, IM_SHIFT = 0x04, IM_NONE = 0x00 };
enum mouse_state { MS_LMB_DOWN = 1, MS_LMB_UP = 2, MS_RMB_DOWN = 3, MS_RMB_UP = 4, MS_MMB_DOWN = 5, MS_MMB_UP = 6 };
enum scroll_direction { SD_UP, SD_DOWN, SD_NONE };

static struct gui_window {
	DWORD style;

	HINSTANCE h_instance;
	HWND handle;

	bool quit;
	bool borderless;
	bool visible;
} the_window;

struct gui_mouse {
	v2 position;
	unsigned state;
	scroll_direction scroll_dir;
	bool btn_was_down;
};

static struct gui_input {
	unsigned key;
	unsigned last_key;
	unsigned modifier;
	bool key_was_down;
	bool key_is_down;
	gui_mouse mouse;
} the_input;

static DWORD wnd_styles[] = {
	WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
	WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | 				  WS_MINIMIZEBOX,
	WS_OVERLAPPED | WS_CAPTION | 			  WS_THICKFRAME
};

static bool initialize_opengl (HWND window, v2 wnd_size) {
	ASSERT (window != 0);

	// Create dummy context
	HDC dc = GetDC (window);
	PIXELFORMATDESCRIPTOR format = { };
	format.nSize = sizeof (format);
	format.nVersion = 1;
	format.iPixelType = PFD_TYPE_RGBA;
	format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	format.cColorBits = 32;
	format.cAlphaBits = 8;
	format.iLayerType = PFD_MAIN_PLANE;

	int format_id = ChoosePixelFormat (dc, &format);
	PIXELFORMATDESCRIPTOR actual_format;
	DescribePixelFormat (dc, format_id, sizeof (actual_format), &actual_format);
	SetPixelFormat (dc, format_id, &actual_format);

	HGLRC gl_context = wglCreateContext (dc);
	if (!wglMakeCurrent (dc, gl_context)) {
		io_log_error ("Unable to create OpenGL context in gui_window:::initialize_opengl ()");
		return false;
	}

	// Create proper modern GL context
	WGL_CHOOSEPIXELFORMATARB wglChoosePixelFormatARB = (WGL_CHOOSEPIXELFORMATARB)wglGetProcAddress ("wglChoosePixelFormatARB");
	WGL_CREATECONTEXTATTRIBSARB wglCreateContextAttribsARB = (WGL_CREATECONTEXTATTRIBSARB)wglGetProcAddress ("wglCreateContextAttribsARB");

	const int attrib_list[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		0
	};

	unsigned format_count;
	wglChoosePixelFormatARB (dc, attrib_list, NULL, 1, &format_id, &format_count);
	DescribePixelFormat (dc, format_id, sizeof (actual_format), &actual_format);
	SetPixelFormat (dc, format_id, &actual_format);

	const int ctx_attrib_list[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 1,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,	
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	gl_context = wglCreateContextAttribsARB (dc, NULL, ctx_attrib_list);
	if (!wglMakeCurrent (dc, gl_context)) {
		io_log_error ("Unable to create OpenGL context in gui_window:::initialize_opengl ()");
		return false;
	}

	ReleaseDC (window, dc);

	return true;
}

static void update_input () {
	the_input.modifier = IM_NONE;
	if (the_input.key_is_down)
		the_input.key_was_down = true;
	else if (the_input.key == 0 && the_input.key_was_down) {
		the_input.key_was_down = false;
		the_input.last_key = 0;
	}

	if (the_input.mouse.state == MS_LMB_DOWN ||
		the_input.mouse.state == MS_RMB_DOWN ||
		the_input.mouse.state == MS_MMB_DOWN) {
		the_input.mouse.btn_was_down = true;
	}
	else if (the_input.mouse.btn_was_down && 
			 (the_input.mouse.state == MS_LMB_UP ||
			  the_input.mouse.state == MS_RMB_UP || 
			  the_input.mouse.state == MS_MMB_UP)) {
		the_input.mouse.btn_was_down = false;
	}

	the_input.mouse.scroll_dir = SD_NONE;
}

static LRESULT CALLBACK window_proc (HWND window, UINT msg, WPARAM w_param, LPARAM l_param) {
	switch (msg) {
		case WM_CLOSE: {
			the_window.quit = true;
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage (0);
			return 0;
		}
		case WM_NCCALCSIZE: {
			if (the_window.borderless)
				return 0;

			break;
		}
	}

	return DefWindowProc (window, msg, w_param, l_param);
}

void wnd_create (const char* title, unsigned width, unsigned height, bool borderless) {
	if (!title)
		wnd_die_gracefully ("Unable create a window with no title in gui_window::wnd_create ()");

	if (width <= 0 || height <= 0)
		io_log_warning ("Trying to create a window with %dx%d dimensions. This might not be desirable.", width, height);

	the_window = { };
	the_window.h_instance = GetModuleHandle (0);
	the_window.borderless = borderless;

	WNDCLASS wnd_class = { };
	wnd_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd_class.lpfnWndProc = window_proc;
	wnd_class.hInstance = the_window.h_instance;
	wnd_class.lpszClassName = title;

	if (RegisterClass (&wnd_class)) {
		the_window.handle = CreateWindow (title, title, wnd_styles[S_DEFAULT], 
										  CW_USEDEFAULT, CW_USEDEFAULT,
										  width, height, 
									      0, 0, the_window.h_instance, 0);

		v2 size = wnd_get_size ();
		v2 client_size = wnd_get_client_size ();

		v2 difference = make_v2 (size.x - client_size.x, size.y - client_size.y);

		SetWindowPos (the_window.handle, 0, 0, 0, width + (unsigned)difference.x, height + (unsigned)difference.y, 0);
		
		HCURSOR default_cursor = LoadCursor (NULL, IDC_ARROW);
		SetCursor (default_cursor);

		if (the_window.handle) {
			if (!initialize_opengl (the_window.handle, make_v2 (width, height)))
				wnd_die_gracefully ("Unable to initialize OpenGL in gui_window::wnd_create ()");
		}
		else
			wnd_die_gracefully ("Unable to create the window in gui_window::wnd_create ()");
	}
	else
		wnd_die_gracefully ("Unable to register window class in gui_window::wnd_create ()");
}

void wnd_create_console () {
	AllocConsole ();
}

bool wnd_update () {
	if (!the_window.visible) {
		ShowWindow (the_window.handle, SW_SHOW);
		the_window.visible = true;
	}

	if (the_window.quit)
		return false;

	update_input ();

	MSG msg;
	if (PeekMessage (&msg, the_window.handle, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				if (GetKeyState (K_CTRL) & 0x8000)
					the_input.modifier |= IM_CTRL;
				if (GetKeyState (K_SHIFT) & 0x8000)
					the_input.modifier |= IM_SHIFT;
				if (GetKeyState (K_ALT) & 0x8000)
					the_input.modifier |= IM_ALT;

				if (msg.wParam == K_F4) {
					if (the_input.modifier & IM_ALT)
						the_window.quit = true;
				}

				unsigned last_key = the_input.key;
				the_input.key = (unsigned)msg.wParam;
				if (last_key != the_input.key) {
					the_input.key_is_down = true;
					the_input.key_was_down = false;
				}

				break;
			}
			case WM_SYSKEYUP:
			case WM_KEYUP: {
				the_input.key = 0;
				the_input.last_key = (unsigned)msg.wParam;
				the_input.key_is_down = false;

				break;
			}
			case WM_LBUTTONDOWN: {
				the_input.mouse.state = MS_LMB_DOWN;
				the_input.mouse.btn_was_down = false;	
				break;
			}
			case WM_RBUTTONDOWN: {
				the_input.mouse.state = MS_RMB_DOWN;
				the_input.mouse.btn_was_down = false;	
				break;	
			}
			case WM_MBUTTONDOWN: {
				the_input.mouse.state = MS_MMB_DOWN;
				the_input.mouse.btn_was_down = false;	
				break;
			}
			case WM_LBUTTONUP: {
				the_input.mouse.state = MS_LMB_UP;
				break;
			}
			case WM_RBUTTONUP: {
				the_input.mouse.state = MS_RMB_UP;
				break;
			}
			case WM_MBUTTONUP: {
				the_input.mouse.state = MS_MMB_UP;
				break;
			}
			case WM_MOUSEWHEEL: {
				int delta = GET_WHEEL_DELTA_WPARAM (msg.wParam);
				the_input.mouse.scroll_dir = delta < 0 ? SD_DOWN : SD_UP;
			}
			default: {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	POINT mouse_pos;
	GetCursorPos (&mouse_pos);
	ScreenToClient (the_window.handle, &mouse_pos);
	the_input.mouse.position.x = (float)mouse_pos.x;
	the_input.mouse.position.y = (float)mouse_pos.y;

	v2 wnd_size = wnd_get_size ();
	glViewport (0, 0, (int)wnd_size.x, (int)wnd_size.y);
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);

	return true;
}

void wnd_close () {
	FreeConsole ();
}

void wnd_die_gracefully (const char* text, ...) {
	char message[128];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, text);
	written = _vsnprintf_s (message, sizeof (message) - 1, text, arguments);
	va_end (arguments);

	int result = MessageBox (the_window.handle, 
							 message, 
							 NULL,
							 MB_OK | MB_ICONERROR);

	if (result == IDOK)
		exit (EXIT_FAILURE);
}

void wnd_swap_buffers () {
	HDC dc = GetDC (the_window.handle);
	SwapBuffers (dc);
	ReleaseDC (the_window.handle, dc);
}

void wnd_set_style (wnd_style style) {
	if (style >= ARRAY_SIZE (wnd_styles)) {
		io_log_error ("Such style does not exist. The style of the window will not be changed");
		return;
	}

	SetWindowLongPtr (the_window.handle, GWL_STYLE, wnd_styles[style]);
}

v2 wnd_get_size () {
	RECT rect;
	GetWindowRect (the_window.handle, &rect);

	return make_v2 (rect.right - rect.left, rect.bottom - rect.top);
}

v2 wnd_get_client_size () {
	RECT rect;
	GetClientRect (the_window.handle, &rect);

	return make_v2 (rect.right, rect.bottom);
}

bool input_is_key_down (unsigned code) {
	// Returns true the moment the key is pressed
	bool result = the_input.key == code;
	return !(!result || the_input.key_was_down);
}

bool input_is_key_pressed (unsigned code) {
	// Returns true while the key is being held
	switch (code) {
		case K_CTRL:
			return the_input.modifier & IM_CTRL;
		case K_SHIFT:
			return the_input.modifier & IM_SHIFT;
		case K_ALT:
			return the_input.modifier & IM_ALT;
		default: {
			bool result = the_input.key == code;
			if (!result || !the_input.key_is_down)
				return false;

			return true;
		}
	}
}

bool input_is_key_up (unsigned code) {
	// Returns true the moment the key is released
	bool result = the_input.last_key == code;
	return !(!result || !the_input.key_was_down);
}

bool input_is_mb_down (unsigned btn) {
	unsigned state = 0;
	switch (btn) {
		case M_LEFT: {
			state = MS_LMB_DOWN;
			break;
		}
		case M_RIGHT: {
			state = MS_RMB_DOWN;
			break;
		}
		case M_MIDDLE: {
			state = MS_MMB_DOWN;	
			break;
		}
	}

	bool result = the_input.mouse.state == state;
	return !(!result || the_input.mouse.btn_was_down);
}

bool input_is_mb_pressed (unsigned btn) {
	unsigned state = 0;
	switch (btn) {
		case M_LEFT: {
			state = MS_LMB_DOWN;
			break;
		}
		case M_RIGHT: {
			state = MS_RMB_DOWN;
			break;
		}
		case M_MIDDLE: {
			state = MS_MMB_DOWN;	
			break;
		}
	}

	bool result = the_input.mouse.state == state;
	return result && the_input.mouse.btn_was_down;
	// return !(!result || the_input.mouse.btn_was_down);
}

bool input_is_mb_up (unsigned btn) {
	unsigned state = 0;
	switch (btn) {
		case M_LEFT: {
			state = MS_LMB_UP;
			break;
		}
		case M_RIGHT: {
			state = MS_RMB_UP;
			break;
		}
		case M_MIDDLE: {
			state = MS_RMB_UP;	
			break;
		}
	}

	return the_input.mouse.state == state && the_input.mouse.btn_was_down;
}

v2 input_get_mouse_position () {
	return the_input.mouse.position;
}

int input_get_scroll_dir () {
	return the_input.mouse.scroll_dir == SD_DOWN ? 
		-1 : (the_input.mouse.scroll_dir == SD_UP ? 1 : 0);
}