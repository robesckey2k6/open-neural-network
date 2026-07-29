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
	
	unsigned int vao;
	unsigned int vbo;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	float vertexs[1] = {1.0f};

	glBufferData(GL_ARRAY_BUFFER, sizeof(float), vertexs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*) 0);

	glEnableVertexAttribArray(0);

	
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

	printf("VERTEX_SHADER:\n");
	int length = (int)(vertex_shader_file_size/ sizeof(char));
	for (int i = 0; i < length; i++) {
		printf("%c", vertex_shader[i]);
	}
	printf("\n");
	printf("FRAGMENT_SHADER:\n");

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

int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file, const char* output_varying_name) {
	unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, shader_file->vertex_shader);	

	unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, shader_file->fragment_shader);

	

	if(!vertex_shader || !fragment_shader) {
		return 0;
	}
	
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	// TODO: make this a paramter
	const char* feedbackVaryings = {output_varying_name};
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

float* gl_compute(gl_instance* instance, nn_layer* layer, float* data) {
	unsigned int tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(layer->output), nullptr, GL_STATIC_DRAW);
	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
	
	glUseProgram(instance->program);
	
	float input_location = glGetUniformLocation(instance->program, "input_layer");
	float weight_location = glGetUniformLocation(instance->program, "weight_layer");
	
	//TODO: figure out a way to figure out matrix in and out dimentions
	glUniform2fv(input_location, 1, data);
	glUniformMatrix3x2fv(weight_location, 1, GL_FALSE, layer->weights);
	
	
	
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);

	float* result = (float*)malloc(sizeof(float) * layer->output);
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * layer->output, result);
	
	glEndTransformFeedback();
	glFlush();

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

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
	printf("func: %s file: %s line:%s\n", function, file, line);
    }
}

