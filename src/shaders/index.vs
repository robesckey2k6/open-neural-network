#version 430 core
uniform vec2 input_layer;
out float output_layer;

layout(binding = 1, std430) readonly buffer ssbo1{
	float weights[];
};

float sigmoid(float z) {
	return 1.0 / (1.0 + exp(-z)); 
}

void main() {
	int instance = gl_InstanceID;

	float sum = 0.0f;

	for(int i = 0; i < 2; i++) {
		sum += input_layer[i] * weights[instance * 2 + i];
	}

	output_layer = sigmoid(sum);
}
