#if !defined (GUI_STRING_BUFFER_H)
#define GUI_STRING_BUFFER_H

#include "gui_math.h"

struct str_buffer {
	char* text;
	unsigned pre_size;
	unsigned post_size;
	unsigned gap_size;
	unsigned gap_pos;

	// x - position in characters starting from the left
	// y - position in lines starting from the top
	v2 caret;
	v2 selection_start;
	v2 selection_end;
};

str_buffer make_str_buffer (char* text);
char* str_buffer_get_text (str_buffer buf);
char* str_buffer_get_selected_text (str_buffer buf);
unsigned str_buffer_get_text_size (str_buffer buf);
rect str_buffer_get_selection_rect (str_buffer buf);

void str_buffer_insert (str_buffer* buf, char c);
void str_buffer_insert_text (str_buffer* buf, const char* text);
void str_buffer_del_before (str_buffer* buf);
void str_buffer_del_after (str_buffer* buf);
void str_buffer_del_word_before (str_buffer* buf);
void str_buffer_del_word_after (str_buffer* buf);

void str_buffer_move_left (str_buffer* buf, bool select = false);
void str_buffer_move_right (str_buffer* buf, bool select = false);
void str_buffer_move_beginning (str_buffer* buf, bool select = false);
void str_buffer_move_end (str_buffer* buf, bool select = false);
void str_buffer_seek_next_word (str_buffer* buf, bool select = false);
void str_buffer_seek_prev_word (str_buffer* buf, bool select = false);

unsigned str_length (const char* text);
void str_copy (const char* src, char* dest, unsigned start_index, unsigned copy_size);
char* str_concatenate (const char* left, const char* right);
bool str_equal (const char* left, const char* right);
char* str_format (const char* format, ...);

#endif