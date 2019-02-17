#if !defined (GUI_WINDOW_H)
#define GUI_WINDOW_H

#define GUI_MAIN int CALLBACK WinMain (HINSTANCE h_instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)

#include <windows.h>
#include "gui_math.h"

// Key codes
#define K_BACK		0x08
#define K_TAB		0x09
#define K_ENTER		0x0D
#define K_SHIFT		0x10
#define K_CTRL 		0x11
#define K_ALT		0x12
#define K_ESC 		0x1B
#define K_SPACE 	0x20
#define K_PGUP		0x21
#define K_PGDOWN	0x22
#define K_END		0x23
#define K_HOME		0x24
#define K_LEFT		0x25
#define K_UP 		0x26
#define K_RIGHT 	0x27
#define K_DOWN 		0x28
#define K_PRINT		0x2A
#define K_PRNTSCR 	0x2C
#define K_INSERT 	0x2D
#define K_DEL		0x2E
#define K_ALPHA0	0x30
#define K_ALPHA1	0x31
#define K_ALPHA2	0x32
#define K_ALPHA3	0x33
#define K_ALPHA4	0x34
#define K_ALPHA5	0x35
#define K_ALPHA6	0x36
#define K_ALPHA7	0x37
#define K_ALPHA8	0x38
#define K_ALPHA9	0x39
#define K_A 		0x41
#define K_B 		0x42
#define K_C 		0x43
#define K_D 		0x44
#define K_E 		0x45
#define K_F 		0x46
#define K_G 		0x47
#define K_H 		0x48
#define K_I 		0x49
#define K_J 		0x4A
#define K_K 		0x4B
#define K_L 		0x4C
#define K_M 		0x4D
#define K_N 		0x4E
#define K_O 		0x4F
#define K_P 		0x50
#define K_Q 		0x51
#define K_R 		0x52
#define K_S 		0x53
#define K_T 		0x54
#define K_U 		0x55
#define K_V 		0x56
#define K_W 		0x57
#define K_X 		0x58
#define K_Y 		0x59
#define K_Z 		0x5A
#define K_LEFTWIN 	0x5B
#define K_RIGHTWIN 	0x5C
#define K_SLEEP 	0x5F
#define K_NUM0 		0x60
#define K_NUM1 		0x61
#define K_NUM2 		0x62
#define K_NUM3 		0x63
#define K_NUM4 		0x64
#define K_NUM5 		0x65
#define K_NUM6 		0x66
#define K_NUM7 		0x67
#define K_NUM8 		0x68
#define K_NUM9 		0x69
#define K_ASTERISK	0x6A
#define K_PLUS		0x6B
#define K_MINUS 	0x6D
#define K_DECIMAL	0x6E
#define K_SLASH 	0x6F
#define K_F1 		0x70
#define K_F2 		0x71
#define K_F3 		0x72
#define K_F4 		0x73
#define K_F5 		0x74
#define K_F6 		0x75
#define K_F7 		0x76
#define K_F8 		0x77
#define K_F9 		0x78
#define K_F10 		0x79
#define K_F11		0x7A
#define K_F12 		0x7B
#define K_NUMLOCK 	0x90
#define K_SCRLLOCK	0x91 
#define K_COLON 	0xBA
#define K_EQUAL		0xBB
#define K_COMMA		0xBC
#define K_UNDER		0xBD
#define K_PERIOD	0xBE
#define K_QUESTION	0xBF
#define K_TILDE 	0xC0
#define K_LBRACE 	0xDB
#define K_PIPE 		0xDC
#define K_RBRACE 	0xDD
#define K_QUOTE		0xDE

#define M_LEFT		0x01
#define M_RIGHT		0x02
#define M_MIDDLE 	0X03

typedef HCURSOR cursor_id;

enum wnd_style { S_DEFAULT, S_FIXEDSIZE, S_NOBUTTONS, S_COUNT };
enum scroll_direction { SD_UP, SD_DOWN, SD_NONE };

struct gui_mouse {
	v2 position;
	unsigned state;
	scroll_direction scroll_dir;
	bool btn_was_down;
};

struct gui_input {
	unsigned key;
	unsigned last_key;
	unsigned modifier;
	bool key_was_down;
	bool key_is_down;
	gui_mouse mouse;
};

struct gui_window {
	DWORD style;

	HINSTANCE h_instance;
	HWND handle;

	v4 bg_color;
	cursor_id cursor;

	gui_input input;

	bool quit;
	bool borderless;
	bool visible;
};

gui_window wnd_create (const char* title, unsigned width, unsigned height, bool borderless = false);
void wnd_create_console ();
bool wnd_update (gui_window* window);
void wnd_close ();
void wnd_die_gracefully (const char* message, ...);
void wnd_swap_buffers (gui_window window);
void wnd_set_style (gui_window window, wnd_style style);
void wnd_set_title (gui_window window, const char* title, ...);
v2 wnd_get_size (gui_window window);
v2 wnd_get_client_size (gui_window window); 

cursor_id wnd_generate_new_cursor (const char* path_to_cursor);
void wnd_set_cursor (gui_window* window, cursor_id id);
void wnd_set_icon (gui_window window, const char* path_to_icon);

bool input_is_key_down (gui_window window, unsigned code);
bool input_is_key_pressed (gui_window window, unsigned code);
bool input_is_key_up (gui_window window, unsigned code);
bool input_is_mb_down (gui_window window, unsigned btn);
bool input_is_mb_pressed (gui_window window, unsigned btn);
bool input_is_mb_up (gui_window window, unsigned btn);
v2 input_get_mouse_position (gui_window window);
int input_get_scroll_dir (gui_window window);

#endif