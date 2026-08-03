#version 430 core

layout(binding = 1, std430) readonly buffer ssbo1{
	int input_size;
	int output_size;
	
	float data[];
};

out float output_layer;


float sigmoid(float z) {
	return 1.0 / (1.0 + exp(-z)); 
}

void main() {
	int instance = gl_InstanceID;

	float sum = 0.0f;

	for(int i = 0; i < input_size; i++) {
		float in_val = data[i];
		float w = data[(input_size - 1) + instance * input_size + i];
		sum += in_val * w;
	}

	output_layer = sigmoid(sum); 
}
