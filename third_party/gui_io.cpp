#include "gui_io.h"

#include "gui_window.h"
#include <stdio.h>
#include <Shlobj.h>

#include "gui_string_buffer.h"

DWORD open_modes[] = {
	GENERIC_READ,
	GENERIC_WRITE,
	GENERIC_READ | GENERIC_WRITE
};

static void log_to_console (const char* text) {
	HANDLE handle = GetStdHandle (STD_OUTPUT_HANDLE);
	if (handle) {
		DWORD written = 0;
		WriteConsole (handle, text, str_length (text), &written, NULL);

		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo (handle, &info);
		COORD cursor = { 0, info.dwCursorPosition.Y + 1 };
		SetConsoleCursorPosition (handle, cursor);
	}
}

file io_open (const char* path, open_mode mode) {
	file result = { };
	result.handle = CreateFile (path, open_modes[mode],
						   		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
						   		OPEN_ALWAYS,
						   		FILE_ATTRIBUTE_NORMAL, NULL);

	result.path = (char*)path;
	GetFileTime (result.handle, NULL, NULL, &result.last_write_time);

	return result;
}

bool io_read (file* f) {
	DWORD size = GetFileSize (f -> handle, NULL);
	f -> contents = (char*)calloc (size + 1, sizeof (char));

	if (!f -> contents)
		wnd_die_gracefully ("Unable to allocate %d bytes in memory in gui_io::io_read ()", sizeof (char) * (size + 1));

	DWORD bytes;
	bool success = ReadFile (f -> handle, f -> contents, size + 1, &bytes, NULL);
	f -> contents[size] = '\0';

	return success;
}

bool io_read (const char* path, char** buffer) {
	file f = io_open (path, OM_READ);

	if (!f.handle) {
		io_log_error ("Unable to open file %s in gui_io::io_read ()", path);
		return false;
	}

	bool success = io_read (&f);
	if (success) {
		unsigned size = str_length (f.contents);
		*buffer = (char*)malloc (sizeof (char) * (size + 1));
		str_copy (f.contents, *buffer, 0, size);
		(*buffer)[size] = '\0';
	}

	io_close (&f);

	return success;
}

bool io_write (file* f, char* text, write_mode mode) {
	if (!f -> handle) {
		io_log_error ("Handle to the file must not be null in gui_io::io_write ()");
		return false;
	}

	if (!text) {
		io_log_error ("Text is null in gui_io::io_write ()");
		return false;
	}

	if (mode = WM_OVERWRITE) {
		LARGE_INTEGER offset = { };
		SetFilePointerEx (f -> handle, offset, NULL, FILE_BEGIN);
		SetEndOfFile (f -> handle);
	}

	unsigned size = str_length (text);

	DWORD bytes = 0;
	return WriteFile (f -> handle, text, size, &bytes, NULL);
}

bool io_write (const char* path, char* text, write_mode mode) {
	file f = io_open (path, OM_WRITE);
	if (!f.handle) {
		io_log_error ("Unable to open file %s in gui_io::io_write ()", path);
		return false;
	}

	bool success = io_write (&f, text, mode);
	io_close (&f);

	return success;
}

void io_close (file* f) {
	if (!f -> handle) {
		io_log_error ("Unable to close file. Handle is null in gui_io::io_close ()");
		return;
	}

	CloseHandle (f -> handle);
	free (f -> contents);
}

bool io_was_file_changed (file f) {
	FILETIME write_time;

	if (GetFileTime (f.handle, NULL, NULL, &write_time))
		return CompareFileTime (&f.last_write_time, &write_time) < 0;
	else {
		DWORD error = GetLastError ();
		io_log_error ("Unable to get last write time of the file %s in gui_io::io_was_file_changed ()", f.path);
	}

	return false;
}

bool io_show_save_file_dialog (const char* title, const char* file_type, const char* extension, char** result) {
	char* filter = (char*)malloc (sizeof (char) * PATH_MAX);
	unsigned file_type_count = str_length (file_type);
	unsigned extension_count = str_length (extension);

	for (unsigned i = 0; i < file_type_count; ++i)
		filter[i] = file_type[i];

	filter[file_type_count] = ' ';
	filter[file_type_count + 1] = '(';
	filter[file_type_count + 2] = '*';
	filter[file_type_count + 3] = '.';

	for (unsigned i = file_type_count + 4; i < file_type_count + 4 + extension_count; ++i)
		filter[i] = extension[i - file_type_count - 4];

	filter[file_type_count + 4 + extension_count] = ')';
	filter[file_type_count + 5 + extension_count] = '\0';
	filter[file_type_count + 6 + extension_count] = '\0';

	char file_string[PATH_MAX] = "";

	OPENFILENAME open_file_name = { };
	open_file_name.lStructSize = sizeof (OPENFILENAME);
	open_file_name.lpstrFilter = filter;
	open_file_name.lpstrFile = file_string;
	open_file_name.nMaxFile = PATH_MAX;
	open_file_name.lpstrTitle = title;
	open_file_name.Flags = OFN_CREATEPROMPT | OFN_DONTADDTORECENT | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT;
	open_file_name.nFileOffset = 0;
	open_file_name.lpstrDefExt = extension;

	bool res = false;
	if (GetSaveFileName (&open_file_name)) {
		unsigned length = str_length (open_file_name.lpstrFile);
		*result = (char*)malloc (sizeof (char) * (length + 1));
		str_copy (open_file_name.lpstrFile, *result, 0, length);
		(*result)[length] = '\0';
		res = true;
	}

	free (filter);
	return res;
}

bool io_show_load_file_dialog (const char* title, const char* file_type, const char* extension, char** result) {
	char* filter = (char*)malloc (sizeof (char) * PATH_MAX);
	unsigned file_type_count = str_length (file_type);
	unsigned extension_count = str_length (extension);

	for (unsigned i = 0; i < file_type_count; ++i)
		filter[i] = file_type[i];

	filter[file_type_count] = ' ';
	filter[file_type_count + 1] = '(';
	filter[file_type_count + 2] = '*';
	filter[file_type_count + 3] = '.';

	for (unsigned i = file_type_count + 4; i < file_type_count + 4 + extension_count; ++i)
		filter[i] = extension[i - file_type_count - 4];

	filter[file_type_count + 4 + extension_count] = ')';
	filter[file_type_count + 5 + extension_count] = '\0';
	filter[file_type_count + 6 + extension_count] = '\0';

	char file_string[PATH_MAX] = "";

	OPENFILENAME open_file_name = { };
	open_file_name.lStructSize = sizeof (OPENFILENAME);
	open_file_name.lpstrFilter = filter;
	open_file_name.lpstrFile = file_string;
	open_file_name.nMaxFile = PATH_MAX;
	open_file_name.lpstrTitle = title;
	open_file_name.Flags = OFN_CREATEPROMPT | OFN_DONTADDTORECENT | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT;
	open_file_name.nFileOffset = 0;
	open_file_name.lpstrDefExt = extension;

	bool res = false;
	if (GetOpenFileName (&open_file_name)) {
		unsigned length = str_length (open_file_name.lpstrFile);
		*result = (char*)malloc (sizeof (char) * (length + 1));
		str_copy (open_file_name.lpstrFile, *result, 0, length);
		(*result)[length] = '\0';
		res = true;
	}

	free (filter);
	return res;
}

bool io_show_select_folder_dialog (const char* title, char** result) {
	char display_name[PATH_MAX];

	BROWSEINFO info = { };
	info.pszDisplayName = display_name;
	info.lpszTitle = title;
	info.ulFlags = BIF_USENEWUI;

	bool res = false;
	LPITEMIDLIST folder = SHBrowseForFolder (&info);
	if (folder) {
		SHGetPathFromIDList (folder, display_name);

		unsigned length = str_length (display_name);
		*result = (char*)malloc (sizeof (char) * (length + 1));
		str_copy (display_name, *result, 0, length);
		(*result)[length] = '\0';

		res = true;
	}

	return res;
}

void io_log (const char* text, ...) {
	char message[128];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, text);

	written = _vsnprintf_s (message, sizeof (message) - 1, text, arguments);

	if (written > 0)
		log_to_console (message);

	va_end (arguments);
}

void io_log_warning (const char* text, ...) {
	char message[128];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, text);

	written = _vsnprintf_s (message, sizeof (message) - 1, text, arguments);

	if (written > 0) {
		char* warning = str_concatenate ("Warning: ", message);
		log_to_console (warning);

		free (warning);
	}

	va_end (arguments);
}

void io_log_error (const char* text, ...) {
	char message[128];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, text);

	written = _vsnprintf_s (message, sizeof (message) - 1, text, arguments);

	if (written > 0) {
		char* error = str_concatenate ("Error: ", message);
		log_to_console (error);

		free (error);
	}

	va_end (arguments);
}
