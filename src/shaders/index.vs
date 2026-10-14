#version 430 core

layout(binding = 1, std430) readonly buffer ssbo1 {
	float indat[];
};

layout(binding = 2, std430) readonly buffer ssbo2 {
	float weights[];
};

out float outdat;

float sigmoid(float z) {
	return 1.0 / (1.0 + exp(-z)); 
}

void main() {
	int instance = gl_InstanceID;

	float sum = 0.0f;
	int indat_size = indat.length();

	for(int i = 0; i < indat_size; i++) {
		float inval = indat[i];

		int weight_group = instance * indat_size;
		
		float weight = weights[weight_group + i];

		sum += inval * weight;
	}

	outdat = sigmoid(sum); 
}


