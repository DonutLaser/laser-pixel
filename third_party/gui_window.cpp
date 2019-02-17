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

static void update_input (gui_input* input) {
	input -> modifier = IM_NONE;
	if (input -> key_is_down)
		input -> key_was_down = true;
	else if (input -> key == 0 && input -> key_was_down) {
		input -> key_was_down = false;
		input -> last_key = 0;
	}

	if (input -> mouse.state == MS_LMB_DOWN ||
		input -> mouse.state == MS_RMB_DOWN ||
		input -> mouse.state == MS_MMB_DOWN) {
		input -> mouse.btn_was_down = true;
	}
	else if (input -> mouse.btn_was_down && 
			 (input -> mouse.state == MS_LMB_UP ||
			  input -> mouse.state == MS_RMB_UP || 
			  input -> mouse.state == MS_MMB_UP)) {
		input -> mouse.btn_was_down = false;
	}

	input -> mouse.scroll_dir = SD_NONE;
}

static LRESULT CALLBACK window_proc (HWND window, UINT msg, WPARAM w_param, LPARAM l_param) {
	gui_window* wnd = (gui_window*)GetWindowLongPtr (window, GWLP_USERDATA);

	if (wnd != NULL) {
		switch (msg) {
			case WM_CLOSE: {
				wnd -> quit = true;
				return 0;
			}
			case WM_DESTROY: {
				PostQuitMessage (0);
				return 0;
			}
			case WM_NCCALCSIZE: {
				if (wnd -> borderless)
					return 0;

				break;
			}
			case WM_SETCURSOR: {
				SetCursor (wnd -> cursor);
				break;
			}
		}
	}
		
	return DefWindowProc (window, msg, w_param, l_param);
}

gui_window wnd_create (const char* title, unsigned width, unsigned height, bool borderless) {
	if (!title)
		wnd_die_gracefully ("Unable create a window with no title in gui_window::wnd_create ()");

	if (width <= 0 || height <= 0)
		io_log_warning ("Trying to create a window with %dx%d dimensions. This might not be desirable.", width, height);

	gui_window result = { };
	result.h_instance = GetModuleHandle (0);
	result.borderless = borderless;
	result.bg_color = make_color (255, 255, 255, 255);

	WNDCLASS wnd_class = { };
	wnd_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd_class.lpfnWndProc = window_proc;
	wnd_class.hInstance = result.h_instance;
	wnd_class.lpszClassName = title;

	if (RegisterClass (&wnd_class)) {
		result.handle = CreateWindow (title, title, wnd_styles[S_DEFAULT], 
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  width, height, 
									  0, 0, result.h_instance, 0);

		// Calculate the actual size of the window where the specified width and height are the dimensions
		// of the client area alone.
		v2 size = wnd_get_size (result);
		v2 client_size = wnd_get_client_size (result);
		v2 difference = make_v2 (size.x - client_size.x, size.y - client_size.y);

		v2 actual_size = make_v2 (width + difference.x, height + difference.y);

		SetWindowPos (result.handle, 0, 0, 0, (unsigned)actual_size.x, (unsigned)actual_size.y, 0);
		
		HCURSOR default_cursor = LoadCursor (NULL, IDC_ARROW);
		result.cursor = default_cursor;
		SetCursor (result.cursor);

		if (result.handle) {
			if (!initialize_opengl (result.handle, make_v2 (width, height)))
				wnd_die_gracefully ("Unable to initialize OpenGL in gui_window::wnd_create ()");
		}
		else
			wnd_die_gracefully ("Unable to create the window in gui_window::wnd_create ()");
	}
	else
		wnd_die_gracefully ("Unable to register window class in gui_window::wnd_create ()");

	return result;
}

void wnd_create_console () {
	AllocConsole ();
}

bool wnd_update (gui_window* window) {
	if (!window -> visible) {
		ShowWindow (window -> handle, SW_SHOW);
		window -> visible = true;

		// Before showing the window, store the actual window struct for use in window proc
		SetWindowLongPtr (window -> handle, GWLP_USERDATA, (LONG_PTR)window);
	}

	if (window -> quit)
		return false;

	update_input (&(window -> input));

	MSG msg;
	if (PeekMessage (&msg, window -> handle, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				if (GetKeyState (K_CTRL) & 0x8000)
					window -> input.modifier |= IM_CTRL;
				if (GetKeyState (K_SHIFT) & 0x8000)
					window -> input.modifier |= IM_SHIFT;
				if (GetKeyState (K_ALT) & 0x8000)
					window -> input.modifier |= IM_ALT;

				if (msg.wParam == K_F4) {
					if (window -> input.modifier & IM_ALT)
						window -> quit = true;
				}

				unsigned last_key = window -> input.key;
				window -> input.key = (unsigned)msg.wParam;
				if (last_key != window -> input.key) {
					window -> input.key_is_down = true;
					window -> input.key_was_down = false;
				}

				break;
			}
			case WM_SYSKEYUP:
			case WM_KEYUP: {
				window -> input.key = 0;
				window -> input.last_key = (unsigned)msg.wParam;
				window -> input.key_is_down = false;

				break;
			}
			case WM_LBUTTONDOWN: {
				window -> input.mouse.state = MS_LMB_DOWN;
				window -> input.mouse.btn_was_down = false;	
				break;
			}
			case WM_RBUTTONDOWN: {
				window -> input.mouse.state = MS_RMB_DOWN;
				window -> input.mouse.btn_was_down = false;	
				break;	
			}
			case WM_MBUTTONDOWN: {
				window -> input.mouse.state = MS_MMB_DOWN;
				window -> input.mouse.btn_was_down = false;	
				break;
			}
			case WM_LBUTTONUP: {
				window -> input.mouse.state = MS_LMB_UP;
				break;
			}
			case WM_RBUTTONUP: {
				window -> input.mouse.state = MS_RMB_UP;
				break;
			}
			case WM_MBUTTONUP: {
				window -> input.mouse.state = MS_MMB_UP;
				break;
			}
			case WM_MOUSEWHEEL: {
				int delta = GET_WHEEL_DELTA_WPARAM (msg.wParam);
				window -> input.mouse.scroll_dir = delta < 0 ? SD_DOWN : SD_UP;
			}
			default: {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	POINT mouse_pos;
	GetCursorPos (&mouse_pos);
	ScreenToClient (window -> handle, &mouse_pos);
	window -> input.mouse.position.x = (float)mouse_pos.x;
	window -> input.mouse.position.y = (float)mouse_pos.y;

	v2 wnd_size = wnd_get_size (*window);
	glViewport (0, 0, (int)wnd_size.x, (int)wnd_size.y);
	glClearColor (window -> bg_color.r, window -> bg_color.g, window -> bg_color.b, window -> bg_color.a);
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

	int result = MessageBox (GetActiveWindow (), 
							 message, 
							 NULL,
							 MB_OK | MB_ICONERROR);

	if (result == IDOK)
		exit (EXIT_FAILURE);
}

void wnd_swap_buffers (gui_window window) {
	HDC dc = GetDC (window.handle);
	SwapBuffers (dc);
	ReleaseDC (window.handle, dc);
}

void wnd_set_style (gui_window window, wnd_style style) {
	if (style >= ARRAY_SIZE (wnd_styles)) {
		io_log_error ("Such style does not exist. The style of the window will not be changed");
		return;
	}

	SetWindowLongPtr (window.handle, GWL_STYLE, wnd_styles[style]);
}

void wnd_set_title (gui_window window, const char* title, ...) {
	char message[128];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, title);

	written = _vsnprintf_s (message, sizeof (message) - 1, title, arguments);

	if (written > 0)
		SetWindowText (window.handle, message);

	va_end (arguments);
}

v2 wnd_get_size (gui_window window) {
	RECT rect;
	GetWindowRect (window.handle, &rect);

	return make_v2 (rect.right - rect.left, rect.bottom - rect.top);
}

v2 wnd_get_client_size (gui_window window) {
	RECT rect;
	GetClientRect (window.handle, &rect);

	return make_v2 (rect.right, rect.bottom);
}

cursor_id wnd_generate_new_cursor (const char* path_to_cursor) {
	return LoadCursorFromFile (path_to_cursor);
}

void wnd_set_cursor (gui_window* window, cursor_id id) {
	cursor_id actual_cursor = (id == NULL) ? LoadCursor (NULL, IDC_ARROW) : id;
	window -> cursor = actual_cursor;
	SetCursor (actual_cursor);
}

void wnd_set_icon (gui_window window, const char* path_to_icon) {
	HANDLE icon_id = LoadImage (NULL, path_to_icon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	SendMessage (window.handle, WM_SETICON, ICON_BIG, (LPARAM)icon_id);
}

bool input_is_key_down (gui_window window, unsigned code) {
	// Returns true the moment the key is pressed
	bool result = window.input.key == code;
	return !(!result || window.input.key_was_down);
}

bool input_is_key_pressed (gui_window window, unsigned code) {
	// Returns true while the key is being held
	switch (code) {
		case K_CTRL:
			return window.input.modifier & IM_CTRL;
		case K_SHIFT:
			return window.input.modifier & IM_SHIFT;
		case K_ALT:
			return window.input.modifier & IM_ALT;
		default: {
			bool result = window.input.key == code;
			if (!result || !window.input.key_is_down)
				return false;

			return true;
		}
	}
}

bool input_is_key_up (gui_window window, unsigned code) {
	// Returns true the moment the key is released
	bool result = window.input.last_key == code;
	return !(!result || !window.input.key_was_down);
}

bool input_is_mb_down (gui_window window, unsigned btn) {
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

	bool result = window.input.mouse.state == state;
	return !(!result || window.input.mouse.btn_was_down);
}

bool input_is_mb_pressed (gui_window window, unsigned btn) {
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

	bool result = window.input.mouse.state == state;
	return result && window.input.mouse.btn_was_down;
}

bool input_is_mb_up (gui_window window, unsigned btn) {
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

	return window.input.mouse.state == state && window.input.mouse.btn_was_down;
}

v2 input_get_mouse_position (gui_window window) {
	return window.input.mouse.position;
}

int input_get_scroll_dir (gui_window window) {
	return window.input.mouse.scroll_dir == SD_DOWN ? 
		-1 : (window.input.mouse.scroll_dir == SD_UP ? 1 : 0);
}