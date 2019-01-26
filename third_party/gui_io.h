#if !defined (GUI_IO_H)
#define GUI_IO_H

#include <windows.h>

enum write_mode { WM_APPEND, WM_OVERWRITE };
enum open_mode { OM_READ, OM_WRITE, OM_READWRITE };

struct file {
	void* handle;
	char* path;
	FILETIME last_write_time;

	char* contents;
};

file io_open (const char* path, open_mode mode);
bool io_read (file* f);
bool io_read (const char* path, char** buffer);
bool io_write (file* f, char* text, write_mode = WM_OVERWRITE);
bool io_write (const char* path, char* text, write_mode = WM_OVERWRITE);
void io_close (file* f);

bool io_was_file_changed (file f);
bool io_was_file_changed (const char* path);

void io_log (const char* text, ...);
void io_log_warning (const char* text, ...);
void io_log_error (const char* text, ...);

#endif