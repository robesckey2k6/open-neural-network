#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

static const char *vertex_shader_src =
    "#version 330 core\n"
    "in float in_vert;\n"
    "uniform vec2 input_layer;\n"
    "out vec2 output_layer;\n"
    "uniform mat3x2 weight_layer1;\n"
    "uniform mat3x2 weight_layer2;\n"
    
    "vec3 sigmoid(vec3 z) {\n"
	    "return 1.0 / (1.0 + exp(-z));\n"   // GLSL applies exp componentwise!
    "}\n"

    "vec2 sigmoid(vec2 z) {\n"
	    "return 1.0 / (1.0 + exp(-z));\n"   // GLSL applies exp componentwise!
    "}\n"


    "void main() {\n"
    "	vec3 s = input_layer * weight_layer1;\n" // Multiplying matrices aka layer 1 calculation
    " 	vec3 a = sigmoid(s);\n" // Converting returning values through a sigmoid function
    
    "	vec2 s1 = weight_layer2 * a;\n"
    "	vec2 a2 = sigmoid(s1);\n"
    "	output_layer = a2\n;"	
    "}\n";
 
static const char *fragment_shader_src =
    "#version 330 core\n"
    "in vec3 vColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(vColor, 1.0);\n"
    "}\n";


GLuint compile_shader(GLenum type, const char *src) {
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

GLFWwindow* gl_window_create(unsigned int width, unsigned int height) {

	if (!glfwInit()) {
		printf("Failed to init GLFW\n");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(width, height, "NN @ Opengl",
			NULL, NULL);

	if(!window) {
		printf("Couldn't create glfw window");
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return NULL;
	}

	return window;
}

int main(void) {

	GLFWwindow* window = gl_window_create(640, 480);

	if (!window) {
		return -1;
	}

	printf("Opengl version: %s\n", glGetString(GL_VERSION));


	// TODO: Impleent file reader
	// Loading fragment shader
	printf("Loading weights(s)\n");

	float input_layer[2] = {0.85f, 0.25f};

	float weight_layer1[6] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f};

	float weight_layer2[6] = {0.25f, 0.5f, 0.1f, 0.2f, 0.3f, 0.4f};
	
	

	GLuint vs = compile_shader(GL_VERTEX_SHADER,vertex_shader_src); 
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

	if(!vs || !fs) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);


	const char* feedbackVaryings = { "output_layer"};
	glTransformFeedbackVaryings(program, 1, &feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(program);



	GLint linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (!linked) {
		char log[1024];
		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		printf("Program link error:\n%s\n", log);
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glDeleteShader(vs); /* linked into program, no longer needed */
	glDeleteShader(fs);

	
	const float vertices[] = {
		0.0f,
		1.0f
	};

	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// GETTING OUTPUT_LAYER



	int inputAttrib = glGetAttribLocation(program, "in_vert");
	glEnableVertexAttribArray(inputAttrib);
	glVertexAttribPointer(inputAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);


	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), nullptr, GL_STATIC_DRAW);

	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);



	while(!glfwWindowShouldClose(window)) {
		glUseProgram(program); // Binding the shader
		
		// SETTING INPUT MATRIX UNIFORM
		float input_layer_location = glGetUniformLocation(program, "input_layer");
		glUniform2fv(input_layer_location,1,input_layer);

		// SETTING LAYER 1 WEIGHTS
		float weight_layer1_location = glGetUniformLocation(program, "weight_layer1");
		glUniformMatrix3x2fv(weight_layer1_location, 1, GL_FALSE, weight_layer1);

		// SETTING LAYER 2 WEIGHTS
		float weight_layer2_location = glGetUniformLocation(program, "weight_layer2");
		glUniformMatrix3x2fv(weight_layer2_location, 1, GL_FALSE, weight_layer2);



		glClearColor(0.5f, 0.1f, 0.12f, 1.0f); // Clearing the screen just to fucking know if shits working or not
		glClear(GL_COLOR_BUFFER_BIT);

		glBeginTransformFeedback(GL_POINTS);

		glDrawArrays(GL_POINTS, 0, 1);


		GLfloat feedback[2];
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
		glEndTransformFeedback();
		glFlush();

		printf("%f %f\n", feedback[0], feedback[1]);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);	
}

