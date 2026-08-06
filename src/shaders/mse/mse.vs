#version 430 core

layout(binding = 2, std430) readonly buffer ssbo1 {
	int result_size;
	float data[];
};

out float output_layer;

void main() {
	int instance = gl_InstanceID;

	float mse = data[result_size + instance] - data[instance];
	mse = mse * mse;

	output_layer = mse;

}
