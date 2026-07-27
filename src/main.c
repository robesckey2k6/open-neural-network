#include <open_nn/open_nn.h>

int main(void) {
	gl_instance* instance = gl_init();
	gl_shader_file* shaders = gl_read_shaders("./src/shaders/index.vs", "./src/shaders/index.fs");
	gl_compile_shaders(instance, shaders, "output_layer");

	float input_layer[2] = {0.85f, 0.25f};
	float weight_layer1[6] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f};
	float weight_layer2[6] = {0.25f, 0.5f, 0.1f, 0.2f, 0.3f, 0.4f};
	float model_package[2] = {
		0.85f, 0.25f,                          // input_layer
	};

	layer1 = onn_linear(2,3, weights1);
	layer2 = onn_linear(3,2, weights2);
	
	x = layer1(x, weights1);
	x = sigmoid(x);
	y = layer2(x, weights2);

	// Chuck this into opengl instance struct
	unsigned int tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(model_package), nullptr, GL_STATIC_DRAW);
	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

	while(!glfwWindowShouldClose(instance->window)) {
		glUseProgram(instance->program);

		float weight_layer1_location = glGetUniformLocation(instance->program, "weight_layer1");
		float weight_layer2_location = glGetUniformLocation(instance->program, "weight_layer2");
	
	glUniformMatrix3x2fv(weight_layer1_location, 1, GL_FALSE, weight_layer1);
	glUniformMatrix3x2fv(weight_layer2_location, 1, GL_FALSE, weight_layer2);

		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, 14);

		float feedback[2];
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

		glEndTransformFeedback();
		glFlush();

		printf("%f %f\n", feedback[0], feedback[1]);

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glfwSwapBuffers(instance->window);
		glfwPollEvents();
	}

}
