#include "gui_string_buffer.h"

#include <stdlib.h>
#include <stdio.h>

#include "gui_window.h"
#include "gui_math.h"
#include "gui_io.h"

static void internal_del_before (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::internal_del_before ()");
		return;
	}

	--buf -> pre_size;
	++buf -> gap_size;
	--buf -> gap_pos;

	--buf -> caret.x;
}

static void internal_del_after (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::internal_del_after ()");
		return;
	}

	--buf -> post_size;
	++buf -> gap_size;
}

static void set_text (str_buffer* buf, char* text) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::set_text ()");
		return;
	}

	if (!text) {
		io_log_error ("Text is null in gui_string_buffer::set_text ()");
		return;
	}

	buf -> pre_size = str_length (text);

	buf -> text = (char*)malloc (sizeof (char) * (buf -> pre_size + 1));
	if (!buf -> text)
		wnd_die_gracefully ("Unable to allocate %d bytes of memory in gui_string_buffer::set_text ()", sizeof (char) * (buf -> pre_size + 1));

	str_copy (text, buf -> text, 0, buf -> pre_size);

	buf -> text[buf -> pre_size] = '\0';
	buf -> post_size = buf -> gap_size = 0;
	buf -> gap_pos = buf -> pre_size;

	buf -> caret.x = (float)buf -> pre_size;
}

static void del_selection (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::del_selection ()");
		return;
	}

	unsigned count = (unsigned)ABS ((buf -> selection_start.x - buf -> selection_end.x));

	while (count != 0) {
		if (buf -> selection_start.x < buf -> selection_end.x)
			internal_del_before (buf);
		else
			internal_del_after (buf);

		--count;
	}

	buf -> selection_start = buf -> selection_end;
}

static unsigned get_post_start (str_buffer buf) {
	return buf.pre_size + buf.gap_size;
}

static bool is_smth_selected (str_buffer buf) {
	return ABS (buf.selection_start.x - buf.selection_end.x) != 0;
}

str_buffer make_str_buffer (char* text) {
	str_buffer buf = { };
	set_text (&buf, text);

	return buf;
}

char* str_buffer_get_text (str_buffer buf) {
	unsigned size = str_buffer_get_text_size (buf);
	char* result = (char*)calloc (size + 1, sizeof (char));

	if (!result)
		wnd_die_gracefully ("Unable to allocate %d bytes of memory in gui_string_buffer::str_buffer_get_text ()", sizeof (char) * (size + 1));

	if (buf.pre_size != 0)
		str_copy (buf.text, result, 0, buf.pre_size);

	unsigned post_start = get_post_start (buf);
	for (unsigned i = post_start; i < post_start + buf.post_size; ++i)
		result[i - buf.gap_size] = buf.text[i];

	result[size] = '\0';

	return result;
}

char* str_buffer_get_selected_text (str_buffer buf) {
	unsigned count = (unsigned)ABS ((buf.selection_start.x - buf.selection_end.x));
	char* text = str_buffer_get_text (buf);

	char* result = (char*)malloc (sizeof (char) * (count + 1));
	if (!result)
		wnd_die_gracefully ("Unable to allocate %d bytes of memmory in gui_string_buffer::str_buffer_get_selected_text ()", sizeof (char) * (count + 1));

	unsigned start = (unsigned)MIN (buf.selection_start.x, buf.selection_end.x);
	str_copy (text, result, start, count);

	result[count] = '\0';

	return result;
}

unsigned str_buffer_get_text_size (str_buffer buf) {
	return buf.pre_size + buf.post_size;
}

rect str_buffer_get_selection_rect (str_buffer buf) {
	v2 pos = make_v2 (MIN (buf.selection_start.x, buf.selection_end.x),
					  MIN (buf.selection_start.y, buf.selection_end.y));
	v2 size = make_v2 (ABS ((buf.selection_start.x - buf.selection_end.x)),
					   ABS ((buf.selection_start.y - buf.selection_end.y)));

	if (size.y == 0)
		size.y = 1;

	return make_rect (pos, size);
}

void str_buffer_insert (str_buffer* buf, char c) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_insert ()");
		return;
	}

	if (buf -> gap_size == 0) {
		unsigned str_size = str_length (buf -> text);
		unsigned size = str_size == 0 ? 2 : str_size * 2;
		char* new_text = (char*)malloc (sizeof (char) * (size + 1));
		if (!new_text)
			wnd_die_gracefully ("Unable to allocate %d bytes of memory in gui_string_buffer::str_buffer_insert ()", sizeof (char) * (size + 1));

		str_copy (buf -> text, new_text, 0, buf -> pre_size);

		unsigned i = size - buf -> post_size;
		unsigned j = get_post_start (*buf);
		for (i, j; i < size; ++i, ++j) 
			new_text[i] = buf -> text[j];

		new_text[size] = '\0';

		free (buf -> text);
		buf -> text = new_text;
		buf -> gap_size = size - buf -> pre_size - buf -> post_size;
	}

	buf -> text[buf -> pre_size] = c;
	--buf -> gap_size;
	++buf -> pre_size;
	++buf -> gap_pos;

	++buf -> caret.x;
}

void str_buffer_insert_text (str_buffer* buf, const char* text) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_insert ()");
		return;
	}

	while (*text != '\0') {
		str_buffer_insert (buf, *text);
		++text;
	}
}

void str_buffer_del_before (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_del_before ()");
		return;
	}

	if (buf -> pre_size == 0 && !is_smth_selected (*buf))
		return;

	if (is_smth_selected (*buf))
		del_selection (buf);
	else
		internal_del_before (buf);
}

void str_buffer_del_after (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_del_after ()");
		return;
	}

	if (buf -> post_size == 0 && !is_smth_selected (*buf))
		return;

	if (is_smth_selected (*buf))
		del_selection (buf);
	else
		internal_del_after (buf);
}

void str_buffer_del_word_before (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_del_word_before ()");
		return;
	}

	unsigned start_gap_pos = buf -> gap_pos;
	str_buffer_seek_prev_word (buf);

	unsigned count = start_gap_pos - buf -> gap_pos;
	while (count != 0) {
		str_buffer_del_after (buf);
		--count;
	}
}

void str_buffer_del_word_after (str_buffer* buf) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_del_word_after ()");
		return;
	}

	unsigned start_gap_pos = buf -> gap_pos;
	str_buffer_seek_next_word (buf);

	unsigned count = buf -> gap_pos - start_gap_pos;
	while (count != 0) {
		str_buffer_del_before (buf);
		--count;
	}
}

void str_buffer_move_left (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_move_left ()");
		return;
	}

	if (buf -> gap_pos == 0)
		return;

	if (select) {
		if (!is_smth_selected (*buf))
			buf -> selection_start = buf -> selection_end = buf -> caret;

		--buf -> selection_end.x;
	}
	else {
		if (is_smth_selected (*buf))
			buf -> selection_start = buf -> selection_end;
	}

	buf -> text[get_post_start (*buf) - 1] = buf -> text[buf -> pre_size - 1];
	++buf -> post_size;
	--buf -> pre_size;
	--buf -> gap_pos;

	--buf -> caret.x;
}

void str_buffer_move_right (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_move_right ()");
		return;
	}

	if (buf -> post_size == 0)
		return;

	if (select) {
		if (!is_smth_selected (*buf))
			buf -> selection_start = buf -> selection_end = buf -> caret;

		++buf -> selection_end.x;
	}
	else {
		if (is_smth_selected (*buf))
			buf -> selection_start = buf -> selection_end;
	}

	buf -> text[buf -> pre_size] = buf -> text[get_post_start (*buf)];
	++buf -> gap_pos;
	++buf -> pre_size;
	--buf -> post_size;

	++buf -> caret.x;
}

void str_buffer_move_beginning (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_move_beginning ()");
		return;
	}

	while (buf -> pre_size != 0)
		str_buffer_move_left (buf, select);
}

void str_buffer_move_end (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_move_end ()");
		return;
	}

	while (buf -> post_size != 0)
		str_buffer_move_right (buf, select);
}

void str_buffer_seek_next_word (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_seek_next_word ()");
		return;
	}

	while (buf -> post_size != 0) {
		char c1 = buf -> text[buf -> pre_size];
		char c2 = buf -> text[get_post_start (*buf)];

		if (c1 == ' ' && c2 != ' ')
			break;

		str_buffer_move_right (buf, select);
	}
}

void str_buffer_seek_prev_word (str_buffer* buf, bool select) {
	if (!buf) {
		io_log_error ("String buffer is null in gui_string_buffer::str_buffer_seek_prev_word");
		return;
	}

	while (buf -> pre_size != 0) {
		char c1 = buf -> text[buf -> pre_size];
		char c2 = buf -> text[get_post_start (*buf)];

		if (c1 != ' ' && c2 == ' ')
			break;

		str_buffer_move_left (buf, select);
	}
}

unsigned str_length (const char* text) {
	unsigned result = 0;
	if (!text)
		return result;

	while (*text != '\0') {
		++result;
		++text;
	}

	return result;
}

void str_copy (const char* src, char* dest, unsigned start_index, unsigned copy_size) {
	if (!src) {
		io_log_error ("Cannot copy from null string in gui_string_buffer::str_copy ()");
		return;
	}

	if (!dest) {
		io_log_error ("Cannot copy into a null string in gui_string_buffer::str_copy ()");
		return;
	}

	if (copy_size < 0) {
		io_log_error ("Copy size must be positive in gui_string_buffer::str_copy ()");
		return;
	}
	else if (copy_size == 0)
		io_log_warning ("Copy size is 0 in gui_string_buffer::str_copy (). Nothing will be copied into the destination string.");

	for (unsigned i = start_index; i < start_index + copy_size; ++i)
		dest[i - start_index] = src[i];
}

char* str_concatenate (const char* left, const char* right) {
	if (!left || !right) {
		io_log_error ("Cannot concatenate null string in gui_string_buffer::str_concatenate");
		return NULL;
	}

	unsigned length1 = str_length (left);
	unsigned length2 = str_length (right);

	char* result = (char*)malloc (sizeof (char) * (length1 + length2 + 1));
	if (!result)
		wnd_die_gracefully ("Unable to allocate %d bytes in gui_string_buffer::str_concatenate ()", sizeof (char) * (length1 + length2 + 1));

	for (unsigned i = 0; i < length1; ++i)
		result[i] = left[i];

	for (unsigned i = length1; i < length1 + length2; ++i)
		result[i] = right[i - length1];

	result[length1 + length2] = '\0';

	return result;
}

bool str_equal (const char* left, const char* right) {
	if (!left || !right) {
		io_log_error ("Cannot compare null string in gui_string_buffer::str_equal ()");
		return false;
	}

	unsigned length1 = str_length (left);
	unsigned length2 = str_length (right);

	if (length1 != length2)
		return false;

	for (unsigned i = 0; i < length1; ++i) {
		if (left[i] != right[i])
			return false;
	}

	return true;
}

char* str_format (const char* format, ...) {
	char message[512];
	unsigned written = 0;

	va_list arguments;
	va_start (arguments, format);

	written = _vsnprintf_s (message, sizeof (message) - 1, format, arguments);

	if (written > 0) {
		char* result = (char*)malloc (sizeof (char*) * (written + 1));
		str_copy (message, result, 0, written);
		result[written] = '\0';

		return result;
	}

	va_end (arguments);

	return NULL;
}
