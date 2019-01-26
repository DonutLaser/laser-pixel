#include "gui_utility.h"

#include "gui_string_buffer.h"
#include "gui_io.h"
#include "gui_window.h"

static bool is_digit (char c) {
	return c >= 0x30 && c <= 0x39;
}

void utility_copy_to_clipboard (const char* text) {
	if (!text) {
		io_log_error ("Cannot copy null string into the clipboard in gui_utility::utility_copy_to_clipboard ()");
		return;
	}

	int length = str_length (text) + 1;

	HANDLE mem_handle = GlobalAlloc (GMEM_MOVEABLE, length);
	if (!mem_handle) {
		io_log_error ("Unable to get a handle to the memory for copy to clipboard in gui_utility::utility_copy_to_clipboard ()");
		return;
	}

	void* memory = GlobalLock (mem_handle);
	if (!memory) {
		io_log_error ("Unable to access the memory for copy to clipboard in gui_utility::utility_copy_to_clipboard ()");
		return;
	}

	for (int i = 0; i < length; ++i)
		((char*)memory)[i] = text[i];

	GlobalUnlock (mem_handle);

	OpenClipboard (0); {
		EmptyClipboard ();
		SetClipboardData (CF_TEXT, mem_handle);
	}
	CloseClipboard ();
}

char* utility_get_from_clipboard () {
	OpenClipboard (0);
	HANDLE handle = GetClipboardData (CF_TEXT);
	if (!handle) {
		io_log_error ("Unable to get a handle to the memory for getting text from clipboard in gui_utility::utility_get_from_clipboard ()");
		return NULL;
	}

	char* result = (char*)GlobalLock (handle);

	GlobalUnlock (handle);
	CloseClipboard ();

	return result;
}

bool utility_is_integer (const char* text) {
	if (!text) {
		io_log_error ("String is null in gui_utility::utility_is_integer ()");
		return false;
	}

	if (text[0] == '-')
		++text;

	while (*text != '\0') {
		if (!is_digit (*text))
			return false;

		++text;
	}

	return true;
}
char* utility_int_to_string (int value) {
	bool negative = value < 0;
	unsigned count = digit_count (value);
	if (negative)
		++count;

	char* result = (char*)malloc (sizeof (char) * (count + 1)); // +1 for the '\0' character
	if (!result)
		wnd_die_gracefully ("Unable to allocate %d bytes of memory in gui_utility::utility_int_to_string ()", sizeof (char) * (count + 1));

	int i = count - 1; 
	do {
		char c = value % 10;
		result[i--] = ABS (c) + 0x30;
		value /= 10;
	} while (value != 0);

	if (negative)
		result[0] = '-';

	result[count] = '\0';

	return result;
}

int utility_string_to_int (char* txt) {
	int result = 0;

	if (!txt) {
		io_log_error ("String is null in gui_utility::utility_string_to_int ()");
		return result;
	}

	unsigned length = str_length (txt);
	if (!txt || length == 0 || (length > 1 && txt[0] == '0'))
		return result;

	bool negative = txt[0] == '-';

	while (*txt != '\0')  {
		if (*txt != '-')
			result = result * 10 + (*txt - 0x30);

		++txt;
	}

	if (negative)
		result = -result;

	return result;
}
