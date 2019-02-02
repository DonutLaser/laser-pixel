#if !defined (PIXEL_GL_H)
#define PIXEL_GL_H

// Forward declarations
struct rect;
union v4;
union v2;
struct gui_window;
struct gui_image;

void gl_init (gui_window window);

void gl_load_image (gui_image* image);

void gl_begin_clip_rect (v2 window_size, rect clip_rect);
void gl_end_clip_rect ();

void gl_draw_rect (rect r, v4 color);
void gl_draw_image (rect r, v4 color, gui_image image);

#endif