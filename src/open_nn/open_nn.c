#include<open_nn/open_nn.h>

#define WINDOW_H 640
#define WINDOW_W 480

#define OGL_MAJOR_VERSION 3
#define OGL_MINOR_VERSION 3

gl_instance* gl_init() {
	if(!glfwInit()) {
		printf("Failed to init glfw\n");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_H, WINDOW_W, "OPENN @ OPENGL", NULL, NULL);

	if(!window) {
		printf("Couldn't create glfw window\n");
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return NULL;
	}

	gl_instance* instance = (gl_instance*)malloc(sizeof(gl_instance));
	instance->window = window;

	return instance;
}

static long get_file_size(FILE* file) {
	if (!file) {
		printf("Cannot find file in the specified location");
		return 0;
	}

	if(fseek(file, 0, SEEK_END) != 0 ) {
		perror("fseek");
		fclose(file);
		return 0;
	}

	long size = ftell(file);
	if( size < 0) {
		perror("ftell");
		fclose(file);
		return 0;
	}
	
	rewind(file);
	return size;
}

gl_shader_file* gl_read_shaders(const char* vertex_file, const char* fragment_file) {
	FILE *fvertex_shader;
	FILE *ffragment_shader;

	fvertex_shader = fopen(vertex_file, "r");
	ffragment_shader = fopen(fragment_file, "r");
	
	long vertex_shader_file_size = get_file_size(fvertex_shader);
	long fragment_shader_file_size = get_file_size(ffragment_shader);

	const char* vertex_shader = (const char*)malloc(vertex_shader_file_size);
	const char* fragment_shader = (const char*)malloc(fragment_shader_file_size);

	if ( !(vertex_shader) || !(fragment_shader) ) {
		printf("Malloc failed @ gl_read_shaders");
	}
	
	fread(vertex_shader, 1, vertex_shader_file_size, fvertex_shader);

	fread(fragment_shader, 1, fragment_shader_file_size, ffragment_shader);

	gl_shader_file* shader_files = (gl_shader_file*)malloc(sizeof(gl_shader_file));	

	shader_files->vertex_shader = vertex_shader;
	shader_files->fragment_shader = fragment_shader;
	return shader_files;
}



static unsigned int compile_shader(GLenum type, const char *src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		printf("Shader compile error (%s):\n %s\n",
				type == GL_VERTEX_SHADER? "vertex": "fragment", log);
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file) {
	unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, shader_file->vertex_shader);	

	unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, shader_file->fragment_shader);

	if(!vertex_shader || !fragment_shader) {
		return 0;
	}
	
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	// TODO: make this a paramter
	const char* feedbackVaryings = {"output_layer"};
	glTransformFeedbackVaryings(program, 1, &feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(program);
	
	unsigned int linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if(!linked) {
		char log[1024];
		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		printf("Program link error:\n %s\n", log);
		return 0;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	instance->program = program;
	return 1;
	
}
