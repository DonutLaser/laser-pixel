#include "pixel_gl.h"

#include "../third_party/gui_gl_ext.h"
#include "../third_party/gui_io.h"
#include "../third_party/gui_math.h"
#include "../third_party/gui_window.h"
#include "../third_party/gui_resources.h"

#include <gl/gl.h>

static unsigned the_vao;
static unsigned the_shader_color;
static unsigned the_shader_texture;

static bool is_shader_compiled_successfully (unsigned shader, char* buffer, unsigned buffer_size) {
	int success;
	glGetShaderiv (shader, GL_COMPILE_STATUS, &success);
	if (!success)
		glGetShaderInfoLog (shader, buffer_size, NULL, buffer);

	return success;
}

static unsigned load_shader (const char* vert_src, const char* frag_src, gui_window window) {
	char* vert_shader_src = NULL;
	if (!io_read (vert_src, &vert_shader_src)) {
		io_log_error ("Unable to open file %s in gui_internal::load_shader ()", vert_src);
		return -1;
	}

	char* frag_shader_src = NULL;
	if (!io_read (frag_src, &frag_shader_src)) {
		io_log_error ("Unable to open file %s in gui_internal::load_shader ()", frag_src);
		return -1;
	}

	unsigned vert_shader;
	vert_shader = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vert_shader, 1, (const char**)(&vert_shader_src), NULL);
	glCompileShader (vert_shader);
	char log[512];
	if (!is_shader_compiled_successfully (vert_shader, log, 512))
		io_log_error ("Vertex shader compilation failed: %s", log);

	unsigned frag_shader;
	frag_shader = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (frag_shader, 1, (const char**)(&frag_shader_src), NULL);
	glCompileShader (frag_shader);
	if (!is_shader_compiled_successfully (frag_shader, log, 512))
		io_log_error ("Fragment shader compilation failed: %s", log);

	unsigned result = glCreateProgram ();
	glAttachShader (result, vert_shader);
	glAttachShader (result, frag_shader);
	glLinkProgram (result);

	if (result == 0) {
		io_log_error ("Shader program cannot be linked");
		return -1;
	}

	glDeleteShader (vert_shader);
	glDeleteShader (frag_shader);

	free (vert_shader_src);
	free (frag_shader_src);

	glUseProgram (result);
	v2 wnd_size = wnd_get_size (window);
	v2 wnd_client_size = wnd_get_client_size (window);
	m4 ortho = make_ortho (0.0f, (float)wnd_size.x, -(wnd_size.y - wnd_client_size.y), (float)wnd_client_size.y, -1.0f, 1.0f);
	int proj_location = glGetUniformLocation (result, "projection");
	glUniformMatrix4fv (proj_location, 1, GL_FALSE, (float*)ortho.value);

	return result;
}

static void load_extensions () {
	glGenVertexArrays = (GL_GENVERTEXARRAYS)load_method ("glGenVertexArrays");
	glBindVertexArray = (GL_BINDVERTEXARRAY)load_method ("glBindVertexArray");
	glGenBuffers = (GL_GENBUFFERS)load_method ("glGenBuffers");
	glBindBuffer = (GL_BINDBUFFER)load_method ("glBindBuffer");
	glBufferData = (GL_BUFFERDATA)load_method ("glBufferData");
	glBufferSubData = (GL_BUFFERSUBDATA)load_method ("glBufferSubData");
	glEnableVertexAttribArray = (GL_ENABLEVERTEXATTRIBARRAY)load_method ("glEnableVertexAttribArray");
	glVertexAttribPointer = (GL_VERTEXATTRIBPOINTER)load_method ("glVertexAttribPointer");
	glCreateShader = (GL_CREATESHADER)load_method ("glCreateShader");
	glShaderSource = (GL_SHADERSOURCE)load_method ("glShaderSource");
	glCompileShader = (GL_COMPILESHADER)load_method ("glCompileShader");
	glCreateProgram = (GL_CREATEPROGRAM)load_method ("glCreateProgram");
	glAttachShader = (GL_ATTACHSHADER)load_method ("glAttachShader");
	glLinkProgram = (GL_LINKPROGRAM)load_method ("glLinkProgram");
	glDeleteShader = (GL_DELETESHADER)load_method ("glDeleteShader");
	glUseProgram = (GL_USEPROGRAM)load_method ("glUseProgram");
	glGetUniformLocation = (GL_GETUNIFORMLOCATION)load_method ("glGetUniformLocation");
	glUniform3f = (GL_UNIFORM3F)load_method ("glUniform3f");
	glUniform4f = (GL_UNIFORM4F)load_method ("glUniform4f");
	glUniform1i = (GL_UNIFORM1I)load_method ("glUniform1i");
	glUniformMatrix4fv = (GL_UNIFORMMATRIX4FV)load_method ("glUniformMatrix4fv");
	glGetShaderiv = (GL_GETSHADERIV)load_method ("glGetShaderiv");
	glGetShaderInfoLog = (GL_GETSHADERINFOLOG)load_method ("glGetShaderInfoLog");
	glActiveTexture = (GL_ACTIVETEXTURE)load_method ("glActiveTexture");
	glGenerateMipmap = (GL_GENERATEMIPMAP)load_method ("glGenerateMipmap");
}

void gl_init (gui_window window) {
	load_extensions ();

	unsigned vbo;
	float vertices[] = {
		// Top triangle
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,

		// Bottom triangle
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays (1, &the_vao);
	glGenBuffers (1, &vbo);

	glBindVertexArray (the_vao);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof (float), (void*)0);
	glEnableVertexAttribArray (0);

	glBindBuffer (GL_ARRAY_BUFFER, 0);
	glBindVertexArray (0);

	the_shader_color = load_shader ("W:\\pixel\\data\\shaders\\color.vert",
									"W:\\pixel\\data\\shaders\\color.frag", window);
	the_shader_texture = load_shader ("W:\\pixel\\data\\shaders\\texture.vert",
									  "W:\\pixel\\data\\shaders\\texture.frag", window);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void gl_load_image (gui_image* image) {
	glGenTextures (1, &image -> id);
	glBindTexture (GL_TEXTURE_2D, image -> id);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, (int)image -> size.x, 
				  (int)image -> size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image -> data);

	glBindTexture (GL_TEXTURE_2D, 0);
}

void gl_draw_rect (rect r, v4 color) {
	glUseProgram (the_shader_color);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m4 model = make_identity ();
	model = translate (model, make_v3 (r.x, r.y, 0.0f));
	model = scale (model, make_v3 (r.width, r.height, 1.0f));

	int model_loc = glGetUniformLocation (the_shader_color, "model");
	glUniformMatrix4fv (model_loc, 1, GL_TRUE, (float*)model.value);
	int col_loc = glGetUniformLocation (the_shader_color, "color");
	glUniform4f (col_loc, color.r, color.g, color.b, color.a);

	glBindVertexArray (the_vao);
	glDrawArrays (GL_TRIANGLES, 0, 6);

	glDisable (GL_BLEND);
	glBindVertexArray (0);
}

void gl_draw_image (rect r, v4 color, gui_image image) {
	glUseProgram (the_shader_texture);

	m4 model = make_identity ();
	model = translate (model, make_v3 (r.x, r.y, 0.0f));
	model = scale (model, make_v3 (image.size, 1.0f));

	int model_loc = glGetUniformLocation (the_shader_texture, "model");
	glUniformMatrix4fv (model_loc, 1, GL_TRUE, (float*)model.value);
	int col_loc = glGetUniformLocation (the_shader_texture, "color");
	glUniform4f (col_loc, color.r, color.g, color.b, color.a);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, image.id);
	glGenerateMipmap (GL_TEXTURE_2D); // No clue why this has to be called to show the texture properly

	glBindVertexArray (the_vao);
	glDrawArrays (GL_TRIANGLES, 0, 6);
	glBindVertexArray (0);

	glDisable (GL_BLEND);
	glBindTexture (GL_TEXTURE_2D, 0);
}
