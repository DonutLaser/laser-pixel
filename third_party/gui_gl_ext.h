#if !defined (GUI_GL_EXT)
#define GUI_GL_EXT

#define GL_ARRAY_BUFFER    		0x8892
#define GL_CLAMP_TO_EDGE		0x812F
#define GL_STATIC_DRAW     		0x88E4
#define GL_DYNAMIC_DRAW			0x88E8
#define GL_VERTEX_SHADER   		0x8B31
#define GL_FRAGMENT_SHADER 		0x8B30
#define GL_COMPILE_STATUS		0x8B81
#define GL_TEXTURE0 			0x84C0
#define GL_R8					0x8229
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46

typedef void (*GL_GENVERTEXARRAYS) (int n, unsigned *arrays); // glGenVertexArrays ()
typedef void (*GL_BINDVERTEXARRAY) (unsigned array); // glBindVertexArray ()
typedef void (*GL_GENBUFFERS) (int n, unsigned *buffers); // glGenBuffers ()
typedef void (*GL_BINDBUFFER) (unsigned target, unsigned buffer); // glBindBuffer ()
typedef void (*GL_BUFFERDATA) (unsigned target, int size, const void* data, unsigned usage); // glBufferData ()
typedef void (*GL_BUFFERSUBDATA) (unsigned target, int size, const void* data); // glBufferSubData ()
typedef void (*GL_ENABLEVERTEXATTRIBARRAY) (unsigned index); // glEnableVertexAttribArray
typedef void (*GL_VERTEXATTRIBPOINTER) (unsigned index, int size, unsigned type, bool normalized, int stride, const void* pointer); // glVertexAttribPointer
typedef unsigned (*GL_CREATESHADER) (unsigned shader_type); // glCreateShader ()
typedef void (*GL_SHADERSOURCE) (unsigned shader, int count, const char** string, int* length); // glShaderSource ()
typedef void (*GL_COMPILESHADER) (unsigned shader); // glCompileShader ()
typedef unsigned (*GL_CREATEPROGRAM) (); // glCreateProgram ()
typedef void (*GL_ATTACHSHADER) (unsigned program, unsigned shader); // glAttachShader ()
typedef void (*GL_LINKPROGRAM) (unsigned program); // glLinkProgram ()
typedef void (*GL_DELETESHADER) (unsigned shader); // glDeleteShader ()
typedef void (*GL_USEPROGRAM) (unsigned program); // glUseProgram ()
typedef int (*GL_GETUNIFORMLOCATION) (unsigned program, const char* name); // glGetUniformLocation ()
typedef void (*GL_UNIFORM3F) (int location, float v0, float v1, float v2); // glUniform3f ()
typedef void (*GL_UNIFORM4F) (int location, float v0, float v1, float v2, float v3); // glUniform4f ()
typedef void (*GL_UNIFORM1I) (int location, int v0); // glUniform4i ()
typedef void (*GL_UNIFORMMATRIX4FV) (int location, int count, bool transpose, float* value); // glUniformMatrix4fv ()
typedef void (*GL_GETSHADERIV) (unsigned shader, unsigned pname, int* params); // glGetShaderiv ()
typedef void (*GL_GETSHADERINFOLOG) (unsigned shader, int max_length, int* length, char* info_log); // glGetShaderInfoLog ()
typedef void (*GL_ACTIVETEXTURE) (unsigned texture); // glActiveTexture ()
typedef void (*GL_GENERATEMIPMAP) (unsigned target); // glGenerateMipmap ()

static GL_GENVERTEXARRAYS glGenVertexArrays;
static GL_BINDVERTEXARRAY glBindVertexArray;
static GL_GENBUFFERS glGenBuffers;
static GL_BINDBUFFER glBindBuffer;
static GL_BUFFERDATA glBufferData;
static GL_BUFFERSUBDATA glBufferSubData;
static GL_ENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
static GL_VERTEXATTRIBPOINTER glVertexAttribPointer;
static GL_CREATESHADER glCreateShader;
static GL_SHADERSOURCE glShaderSource;
static GL_COMPILESHADER glCompileShader;
static GL_CREATEPROGRAM glCreateProgram;
static GL_ATTACHSHADER glAttachShader;
static GL_LINKPROGRAM glLinkProgram;
static GL_DELETESHADER glDeleteShader;
static GL_USEPROGRAM glUseProgram;
static GL_GETUNIFORMLOCATION glGetUniformLocation;
static GL_UNIFORM3F glUniform3f;
static GL_UNIFORM4F glUniform4f;
static GL_UNIFORM1I glUniform1i;
static GL_UNIFORMMATRIX4FV glUniformMatrix4fv;
static GL_GETSHADERIV glGetShaderiv;
static GL_GETSHADERINFOLOG glGetShaderInfoLog;
static GL_ACTIVETEXTURE glActiveTexture;
static GL_GENERATEMIPMAP glGenerateMipmap;

void* load_method (const char* name);

#endif