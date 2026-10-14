#include<open_nn/open_nn.h>

#define WINDOW_H 640
#define WINDOW_W 480

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 3

#define SHDR_FPS_LOC "./src/shaders/index.vs"
#define SHDR_MSE_LOC "./src/shaders/mse/mse.vs"
#define SHDR_GDE_LOC "./src/shaders/gd/gd.vs"

#define VAR_OUT "outdat"

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
	
	// Generating vao :(
	unsigned int vao;
	glc(glGenVertexArrays(1, &vao));
	glc(glBindVertexArray(vao));

	// Loading shaders
	gl_shader_file* shdrf_fps = gl_read_vshdr(SHDR_FPS_LOC);
	gl_shader_file* shdrf_mse = gl_read_vshdr(SHDR_MSE_LOC);
	gl_shader_file* shdrf_gde = gl_read_vshdr(SHDR_GDE_LOC);

	// Compiling shaders
	int shdr_fps = gl_compile_vertex_shader(shdrf_fps, VAR_OUT);
	int shdr_mse = gl_compile_vertex_shader(shdrf_mse, VAR_OUT);
	int shdr_gde = gl_compile_vertex_shader(shdrf_gde, VAR_OUT);
	
	// Creating gl instance
	gl_instance* instance = (gl_instance*)malloc(sizeof(gl_instance));
	instance->window = window;
	instance->vao = vao;
	instance->shdr_fps = shdr_fps;
	instance->shdr_mse = shdr_mse;
	instance->shdr_gde = shdr_gde;

	return instance;
}

static long get_file_size(FILE* file) {
	if (!file) {
		printf("Cannot find file in the specified location\n");
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

gl_shader_file* gl_read_vshdr(const char* vertex_file) {
	FILE *fvertex_shader;

	fvertex_shader = fopen(vertex_file, "r");
	
	if (!fvertex_shader) {
		printf("ERROR: shader %s cannot be read.\n", fvertex_shader);
		return NULL;
	}
	
	
	long vertex_shader_file_size = get_file_size(fvertex_shader);

	if (vertex_shader_file_size == 0 ) {
		printf("ERROR: unable to get shader file size\n");
	}

	const char* vertex_shader = (const char*)malloc(vertex_shader_file_size);
	
	if ( !(vertex_shader) ) {
		printf("Malloc failed @ gl_read_shaders");
	}
	
	size_t vertex_file_read = fread(vertex_shader, 1, vertex_shader_file_size, fvertex_shader);
	if (vertex_file_read != (size_t) vertex_shader_file_size) {
		 printf("fread failed or short read: expected %ld, got %zu\n", vertex_shader_file_size, vertex_file_read);
		 return NULL;
	}

	gl_shader_file* shader_files = (gl_shader_file*)malloc(sizeof(gl_shader_file));	
	
	if (!shader_files) {
		printf("malloc failed @ shader_files\n");
	}

	shader_files->vertex_shader = vertex_shader;
	shader_files->vertex_shader_size = vertex_shader_file_size;

	return shader_files;
}

gl_shader_file* gl_read_shaders(const char* vertex_file, const char* fragment_file) {
	FILE *fvertex_shader;
	FILE *ffragment_shader;

	fvertex_shader = fopen(vertex_file, "r");
	ffragment_shader = fopen(fragment_file, "r");
	
	if (!fvertex_shader) {
		printf("ERROR: shader %s cannot be read.\n", fvertex_shader);
		return NULL;
	}

	if (!ffragment_shader) {
		printf("ERROR: shader %s cannot be read.\n", ffragment_shader);
		return NULL;
	}
	
	
	long vertex_shader_file_size = get_file_size(fvertex_shader);
	long fragment_shader_file_size = get_file_size(ffragment_shader);

	if (vertex_shader_file_size == 0 || fragment_shader_file_size == 0) {
		printf("ERROR: unable to get shader file size\n");
	}

	const char* vertex_shader = (const char*)malloc(vertex_shader_file_size);
	const char* fragment_shader = (const char*)malloc(fragment_shader_file_size);
	
	if ( !(vertex_shader) || !(fragment_shader) ) {
		printf("Malloc failed @ gl_read_shaders");
	}
	
	size_t vertex_file_read = fread(vertex_shader, 1, vertex_shader_file_size, fvertex_shader);
	size_t fragment_file_read = fread(fragment_shader, 1, fragment_shader_file_size, ffragment_shader);
	if (vertex_file_read != (size_t) vertex_shader_file_size) {
		 printf("fread failed or short read: expected %ld, got %zu\n", vertex_shader_file_size, vertex_file_read);
		 return NULL;
	}

	if (fragment_file_read != (size_t) fragment_shader_file_size) {
		 printf("fread failed or short read: expected %ld, got %zu\n", fragment_shader_file_size, fragment_file_read);
		 return NULL;
	}

	gl_shader_file* shader_files = (gl_shader_file*)malloc(sizeof(gl_shader_file));	
	
	if (!shader_files) {
		printf("malloc failed @ shader_files\n");
	}

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
int gl_compile_vertex_shader(gl_shader_file* shader_file, const char* output_varying_name) {
	unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER,
	                                            shader_file->vertex_shader,
	                                            (int) shader_file->vertex_shader_size);

	if(!vertex_shader) {
		printf("Unable to compile vertex shader\n");
		return 0;
	}

	unsigned int program = glc(glCreateProgram());
	glc(glAttachShader(program, vertex_shader));

	const char* feedback_varyings[] = { output_varying_name };
	glc(glTransformFeedbackVaryings(program, 1, feedback_varyings, GL_INTERLEAVED_ATTRIBS));
	glc(glLinkProgram(program));

	int linked = 0;
	glc(glGetProgramiv(program, GL_LINK_STATUS, &linked));

	if(!linked) {
		char log[1024];
		glc(glGetProgramInfoLog(program, sizeof(log), NULL, log));
		printf("Program link error:\n%s\n", log);

		glc(glDetachShader(program, vertex_shader));
		glc(glDeleteShader(vertex_shader));
		glc(glDeleteProgram(program));
		return 0;
	}

	glc(glDetachShader(program, vertex_shader));
	glc(glDeleteShader(vertex_shader));
	return program;
}

int gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file, const char* output_varying_name) {
	unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, shader_file->vertex_shader, (int) shader_file->vertex_shader_size);	

	unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, shader_file->fragment_shader, (int) shader_file->fragment_shader_size);

	if(!vertex_shader || !fragment_shader) {
		printf("Unable to compile shader vertex: %s fragment: %s\n", !vertex_shader, !fragment_shader);
		return 0;
	}
	
	unsigned int program = glc(glCreateProgram());

	glc(glAttachShader(program, vertex_shader));
	glc(glAttachShader(program, fragment_shader));
	
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

	//instance->program = program;
	return 1;
	
}

float* gl_compute(gl_instance* instance, nn_layer* layer, float* indat) {

	// Input data buffer stuff 
	unsigned int indat_gl_buf;
	glc(glCreateBuffers(1, &indat_gl_buf));
	unsigned int indat_size = layer->input * sizeof(float); 

	// Copying input data -> gpu memory
	glc(glBindBuffer(GL_SHADER_STORAGE_BUFFER, indat_gl_buf));
	glc(glBufferStorage(GL_SHADER_STORAGE_BUFFER, indat_size, indat, GL_DYNAMIC_STORAGE_BIT));

	// Binding the buffer to index	
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indat_gl_buf));
	
	// Weights data buffer stuff
	unsigned int weights_gl_buf;
	glc(glCreateBuffers(1, &weights_gl_buf));
	unsigned int weights_size = ( layer->output * layer->input ) * sizeof(float);

	 // Copying weights data -> gpu memory
	glc(glBindBuffer(GL_SHADER_STORAGE_BUFFER, weights_gl_buf));
	glc(glBufferStorage(GL_SHADER_STORAGE_BUFFER, weights_size, layer->weights, GL_DYNAMIC_STORAGE_BIT));
	
	// Binding buffer to a buffer base index
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, weights_gl_buf));
	
	// Transform feedback buffer object
	unsigned int outdat_gl_buf;
	unsigned int outdat_size = ( layer->output) * sizeof(float);
	float* outdat_buff = (float*)malloc(outdat_size); // NOTE this is for the ouput capturing
	glc(glGenBuffers(1, &outdat_gl_buf));
	
	// Setting up buffer for result form the vertex shader
	glc(glBindBuffer(GL_ARRAY_BUFFER, outdat_gl_buf));
	glc(glBufferData(GL_ARRAY_BUFFER, outdat_size, nullptr, GL_STATIC_READ));
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outdat_gl_buf));
	
	// Loading forwardpass shader
	glc(glUseProgram(instance->shdr_fps));
	
	// Disabling rasterization of opengl
	glc(glEnable(GL_RASTERIZER_DISCARD));

	glc(glBeginTransformFeedback(GL_POINTS));
	glc(glDrawArraysInstanced(GL_POINTS, 0, 1, layer->output));
	glc(glEndTransformFeedback());
	
	// Loading outdat data from the transform feedbakc buffer
	glc(glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outdat_size, outdat_buff));

	return outdat_buff;
}


nn_layer* onn_linear(unsigned int input, unsigned int output, float* weights) {

	nn_layer* layer = (nn_layer*)malloc(sizeof(nn_layer));
	layer->input = input;
	layer->output = output;
	layer->weights = weights;

	return layer;
}

float gl_mse(gl_instance* instance, float* result, float* actual, int count) {
	const size_t bytes = count * sizeof(float);

	// Input buffer 1: network output (binding 1)
	unsigned int result_buf;
	glc(glCreateBuffers(1, &result_buf));
	glc(glNamedBufferStorage(result_buf, bytes, result, 0));
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, result_buf));

	// Input buffer 2: expected values (binding 2)
	unsigned int actual_buf;
	glc(glCreateBuffers(1, &actual_buf));
	glc(glNamedBufferStorage(actual_buf, bytes, actual, 0));
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, actual_buf));

	// Transform feedback buffer: one squared error per instance
	unsigned int tbo;
	glc(glCreateBuffers(1, &tbo));
	glc(glNamedBufferStorage(tbo, bytes, nullptr, 0));
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo));

	// Run
	glc(glUseProgram(instance->shdr_mse));
	glc(glEnable(GL_RASTERIZER_DISCARD));
	glc(glBeginTransformFeedback(GL_POINTS));
	glc(glDrawArraysInstanced(GL_POINTS, 0, 1, count));
	glc(glEndTransformFeedback());
	glc(glDisable(GL_RASTERIZER_DISCARD));

	// Read back
	float* sq_errs = (float*)malloc(bytes);
	glc(glGetNamedBufferSubData(tbo, 0, bytes, sq_errs));

	// Sum on CPU, then apply the 1/2 factor
	float sum = 0.0f;
	for (int i = 0; i < count; i++) {
		sum += sq_errs[i];
	}
	free(sq_errs);

	// Cleanup
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0));
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0));
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0));
	glDeleteBuffers(1, &result_buf);
	glDeleteBuffers(1, &actual_buf);
	glDeleteBuffers(1, &tbo);

	return 0.5f * sum;
}

void gl_clear_error(){
    while(glGetError() != GL_NO_ERROR){
    }
}

#define GD_SSBO_BINDING 1   // must match the binding in the vertex shader

static unsigned int gl_make_ssbo(unsigned int binding, const void* data, size_t bytes) {
	unsigned int buf;
	glc(glCreateBuffers(1, &buf));
	glc(glNamedBufferStorage(buf, bytes, data, 0));
	glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buf));
	return buf;
}

float* gl_gd(
	gl_instance* instance,
	nn_layer*    prev_layer,           // layer2 (3 -> 2)
	float*       prev_layer_input,     // hidden activations (n_hidden floats)
	float*       prev_output,          // output activations (n_out floats)
	float*       expected,             // targets (n_out floats)
	float        learning_rate,
	float*       prev_layer_weights,   // n_hidden * n_out floats
	int*         out_weight_count)
{
	const int    n_hidden     = prev_layer->input;
	const int    n_out        = prev_layer->output;
	const int    n_weights    = n_hidden * n_out;
	const size_t result_bytes = n_weights * sizeof(float);

	// Input SSBOs
	unsigned int hidden_buf = gl_make_ssbo(1, prev_layer_input,   n_hidden  * sizeof(float));
	unsigned int out_buf    = gl_make_ssbo(2, prev_output,        n_out     * sizeof(float));
	unsigned int exp_buf    = gl_make_ssbo(3, expected,           n_out     * sizeof(float));
	unsigned int weight_buf = gl_make_ssbo(4, prev_layer_weights, n_weights * sizeof(float));
	unsigned int lr_buf     = gl_make_ssbo(5, &learning_rate,     sizeof(float));

	// Transform feedback buffer: one new weight per instance
	unsigned int tbo;
	glc(glCreateBuffers(1, &tbo));
	glc(glNamedBufferStorage(tbo, result_bytes, nullptr, 0));
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo));

	// Run
	glc(glUseProgram(instance->shdr_gde));
	glc(glEnable(GL_RASTERIZER_DISCARD));
	glc(glBeginTransformFeedback(GL_POINTS));
	glc(glDrawArraysInstanced(GL_POINTS, 0, 1, n_weights));
	glc(glEndTransformFeedback());
	glc(glDisable(GL_RASTERIZER_DISCARD));

	// Read back
	float* new_weights = (float*)malloc(result_bytes);
	glc(glGetNamedBufferSubData(tbo, 0, result_bytes, new_weights));

	// Cleanup
	glc(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0));
	for (unsigned int b = 1; b <= 5; b++) {
		glc(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, b, 0));
	}
	glDeleteBuffers(1, &hidden_buf);
	glDeleteBuffers(1, &out_buf);
	glDeleteBuffers(1, &exp_buf);
	glDeleteBuffers(1, &weight_buf);
	glDeleteBuffers(1, &lr_buf);
	glDeleteBuffers(1, &tbo);

	if (out_weight_count) *out_weight_count = n_weights;
	return new_weights;   // caller frees
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
