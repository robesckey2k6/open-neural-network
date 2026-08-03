#include<open_nn/open_nn.h>

#define WINDOW_H 640
#define WINDOW_W 480

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 3

void lprintf(const char* fmt, ...) {
	va_list args;

	FILE* fptr = fopen("log.txt", "a");

	va_start(args, fmt);
	int ret = vfprintf(fptr, fmt, args);
	va_end(args);	

	fclose(fptr);
}

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
	
	unsigned int vao;
	unsigned int vbo;

	glc(glGenVertexArrays(1, &vao));
	glc(glGenBuffers(1, &vbo));

	glc(glBindVertexArray(vao));
	glc(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	float vertexs[1] = {1.0f};

	glc(glBufferData(GL_ARRAY_BUFFER, sizeof(float), vertexs, GL_STATIC_DRAW));

	glc(glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*) 0));

	glc(glEnableVertexAttribArray(0));

	
	gl_instance* instance = (gl_instance*)malloc(sizeof(gl_instance));
	instance->window = window;
	instance->vao = vao;
	instance->vbo = vbo;

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

	int length = (int)(vertex_shader_file_size/ sizeof(char));

	shader_files->vertex_shader = vertex_shader;
	shader_files->fragment_shader = fragment_shader;

	shader_files->vertex_shader_size = vertex_shader_file_size;
	shader_files->fragment_shader_size = fragment_shader_file_size;

	return shader_files;
}



static unsigned int compile_shader(GLenum type, const char *src, int length) {
	GLuint shader = glc(glCreateShader(type));

	glc(glShaderSource(shader, 1, &src, &length));
	glc(glCompileShader(shader));

	GLint ok = 0;
	glc(glGetShaderiv(shader, GL_COMPILE_STATUS, &ok));

	if (!ok) {
		char log[1024];
		glc(glGetShaderInfoLog(shader, sizeof(log), NULL, log));
		printf("Shader compile error (%s):\n %s\n",
				type == GL_VERTEX_SHADER? "vertex": "fragment", log);
		glc(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file, const char* output_varying_name) {
	unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, shader_file->vertex_shader, (int) shader_file->vertex_shader_size);	

	unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, shader_file->fragment_shader, (int) shader_file->fragment_shader_size);

	

	if(!vertex_shader || !fragment_shader) {
		return 0;
	}
	
	unsigned int program = glc(glCreateProgram());

	glc(glAttachShader(program, vertex_shader));
	glc(glAttachShader(program, fragment_shader));
	
	// TODO: make this a paramter
	const char* feedbackVaryings = {output_varying_name};

	glc(glTransformFeedbackVaryings(program, 1, &feedbackVaryings, GL_INTERLEAVED_ATTRIBS));

	glc(glLinkProgram(program));
	
	unsigned int linked = 0;
	glc(glGetProgramiv(program, GL_LINK_STATUS, &linked));

	if(!linked) {
		char log[1024];
		glc(glGetProgramInfoLog(program, sizeof(log), NULL, log));
		printf("Program link error:\n %s\n", log);
		return 0;
	}

	glc(glDeleteShader(vertex_shader));
	glc(glDeleteShader(fragment_shader));

	instance->program = program;
	return 1;
	
}

float* gl_compute(gl_instance* instance, nn_layer* layer, float* data) {

	unsigned int compute_payload;
	glCreateBuffers(1, &compute_payload);
	glNamedBufferStorage(compute_payload,
			sizeof(float) * layer->input * layer->output,
			(const void*) layer->weights,
			GL_DYNAMIC_STORAGE_BIT
	);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, compute_payload);

	//TODO: add this to gl instance
	unsigned int tbo;
	glc(glGenBuffers(1, &tbo));
	glc(glBindBuffer(GL_ARRAY_BUFFER, tbo));
	glc(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * layer->output, nullptr, GL_STATIC_DRAW));
	glc(glEnable(GL_RASTERIZER_DISCARD));
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo));
	
	//TODO: bind vao and all everything from instance
	glc(glUseProgram(instance->program));
	
	float input_location = glc(glGetUniformLocation(instance->program, "input_layer"));
	
	//TODO: implement ssbo for this
	glc(glUniform2fv(input_location, 1, data));
	
	
	glc(glBeginTransformFeedback(GL_POINTS));
	glc(glDrawArraysInstanced(GL_POINTS, 0, 1, layer->output));

	float* result = (float*)malloc(sizeof(float) * layer->output);
	
	glc(glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * layer->output, result));
	
	glc(glEndTransformFeedback());
	glc(glFlush());

	glc(glClear(GL_COLOR_BUFFER_BIT));
	glc(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));

	glfwSwapBuffers(instance->window);

	return result;
}


nn_layer* onn_linear(unsigned int input, unsigned int output, float* weights) {

	nn_layer* layer = (nn_layer*)malloc(sizeof(nn_layer));
	layer->input = input;
	layer->output = output;
	layer->weights = weights;

	return layer;
}

void gl_clear_error(){
    while(glGetError() != GL_NO_ERROR){
    }
}

void gl_get_error(const char* function, const char* file, int line) {
    GLenum error = glGetError();
    while(error){
	printf("ERROR: ");
	printf("%x\n", error);
	printf("%s\n", gluErrorString(error));
	printf("func: %s file: %s line:%d\n", function, file, line);
	return;
    }
}
