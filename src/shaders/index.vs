#version 330 core
uniform vec2 input_layer;
uniform mat3x2 weight_layer1;
uniform mat3x2 weight_layer2;

out vec2 output_layer;

vec3 sigmoid(vec3 z) {
	return 1.0 / (1.0 + exp(-z)); 
}
vec2 sigmoid(vec2 z) {
	return 1.0 / (1.0 + exp(-z)); 
}

void main() {
	vec3 s = input_layer * weight_layer1;
 	vec3 a = sigmoid(s); 

	vec2 s1 = weight_layer2 * a;
	vec2 a2 = sigmoid(s1);
	output_layer = a2;
}
