#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include<glad/gl.h>
#include<GL/glu.h>
#include<GLFW/glfw3.h>
#include<string.h>

typedef struct {
	GLFWwindow* window;
	const char* version;
	const char* vertex_shader;
	const char* fragment_shader;
	unsigned int program;

	unsigned int vao;
	unsigned int vbo;
	unsigned int tbo;
} gl_instance;

typedef struct {
	const char* vertex_shader;
	const char* fragment_shader;

	long vertex_shader_size;
	long fragment_shader_size;

} gl_shader_file;

typedef struct {
	unsigned int input;
	unsigned int output;

	float* weights;
} nn_layer;

// * ---------- *
// Gl functions
// * ---------- *
gl_instance* gl_init(); 
int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file, const char* output_varying_name);
gl_shader_file* gl_read_shaders(const char* vertex_file, const char* fragment_file); 
float* gl_compute(gl_instance* instance, nn_layer* layer, float* data);

// * ---------- *
// NN functions
// * ---------- *
nn_layer* onn_linear(unsigned int input, unsigned int output, float* weights);

// * ------------- *
// Helper functions
// * ------------- *
static long get_file_size(FILE* file);
static unsigned int compile_shader(GLenum type, const char *src, int length);
static void gl_get_error(const char* function, const char* file, int line);
static void gl_clear_error();
void lprintf(const char* fmt, ...); 


#define glc(x)\
	(\
	 gl_clear_error(),\
	 x\
	 );\
	gl_get_error(#x, __FILE__, __LINE__);


