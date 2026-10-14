#include <open_nn/open_nn.h>

int main(void) {

	gl_instance* instance = gl_init();

	float input_layer[2] = {0.85f, 0.25f};

	float weight_layer1[6] = {0.1f, 0.4f,
		0.2f, 0.5f,
		0.3f, 0.6f};
	float weight_layer2[6] = {0.25f,  0.1f, 0.3f,
		0.5f, 0.2f, 0.4f};

	float actual_output[2] = {1.0f, 0.0f};

	nn_layer* layer1 = onn_linear(2,3, weight_layer1);
	nn_layer* layer2 = onn_linear(3,2, weight_layer2);

	float* result = gl_compute(instance, layer1, input_layer);
	printf("HIDDEN LAYER: %f, %f, %f\n", result[0], result[1], result[2]);	


	float* result2 = gl_compute(instance, layer2, result);
	printf("HIDDEN LAYER: %f, %f\n", result2[0], result2[1]);	


	float mse_result = gl_mse(instance, result2, actual_output, layer2->output);
	printf("LOSS1: %f\n", mse_result);
	
	int weight_count = 6;

	float* gd = gl_gd(instance,
			layer2,
			result,          // hidden activations (3 floats)
			result2,         // output activations (2 floats)
			actual_output,   // targets (2 floats)
			0.1f,            // learning rate
			weight_layer2,   // hidden -> output weights (6 floats)
			&weight_count);

	printf("NEW WEIGHTS FOR OUTPUT LAYER\n");
	for (int i = 0; i < weight_count; i++) {
		printf("%f\n", gd[i]);
	}
	free(gd);

}
