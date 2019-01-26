#if !defined (GUI_UTILITY_H)
#define GUI_UTILITY_H

#include "gui_window.h"

#define ASSERT(x) if (!(x)) { wnd_die_gracefully ("Assertion failed: %s\nFile: %s\nLine: %d", #x, __FILE__, __LINE__); }

void utility_copy_to_clipboard (const char* text);
char* utility_get_from_clipboard ();

bool utility_is_integer (const char* text);

char* utility_int_to_string (int value);
int utility_string_to_int (char* str);

#endif