#version 330 core
uniform vec2 input_layer;
uniform mat3x2 weight_layer1;

out vec3 output_layer;

vec3 sigmoid(vec3 z) {
	return 1.0 / (1.0 + exp(-z)); 
}
vec2 sigmoid(vec2 z) {
	return 1.0 / (1.0 + exp(-z)); 
}

void main() {
	vec3 s = input_layer * weight_layer1;
 	vec3 a = sigmoid(s); 
	output_layer = a;
}
