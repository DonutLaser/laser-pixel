#if !defined (PIXEL_GL_H)
#define PIXEL_GL_H

// Forward declarations
struct rect;
union v4;

void gl_init ();

void gl_draw_rect (rect r, v4 color);

#endif