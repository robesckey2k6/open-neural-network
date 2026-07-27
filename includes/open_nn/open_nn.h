#include <stdlib.h>
#include <stdio.h>

#include<glad/gl.h>
#include<GLFW/glfw3.h>

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
} gl_shader_file;

typedef struct {
	unsigned int input;
	unsigned int output;
} nn_layer;

// * ---------- *
// Gl functions
// * ---------- *
gl_instance* gl_init(); 
int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file);
gl_shader_file* gl_read_shaders(const char* vertex_file, const char* fragment_file); 


// * ---------- *
// NN functions
// * ---------- *


// * ------------- *
// Helper functions
// * ------------- *
static long get_file_size(FILE* file);
static unsigned int compile_shader(GLenum type, const char *src);




