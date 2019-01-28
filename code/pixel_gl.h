#if !defined (PIXEL_GL_H)
#define PIXEL_GL_H

// Forward declarations
struct rect;
union v4;
struct gui_window;
struct gui_image;

void gl_init (gui_window window);

void gl_load_image (gui_image* image);

void gl_draw_rect (rect r, v4 color);
void gl_draw_image (rect r, v4 color, gui_image image);

#endif