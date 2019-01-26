#if !defined (PIXEL_GL_H)
#define PIXEL_GL_H

// Forward declarations
union v4;

void gl_init ();

void gl_draw_rect (v4 rect, v4 color);

#endif