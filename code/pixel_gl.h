#if !defined (PIXEL_GL_H)
#define PIXEL_GL_H

// Forward declarations
struct rect;
union v4;
struct gui_window;

void gl_init (gui_window window);

void gl_draw_rect (rect r, v4 color);

#endif