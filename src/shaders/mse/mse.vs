#version 430 core

layout(std430, binding = 1) readonly buffer ResultBuf { float result[]; };
layout(std430, binding = 2) readonly buffer ActualBuf { float actual[]; };

out float outdat;

void main() {
	int i = gl_InstanceID;
	float d = result[i] - actual[i];
	outdat = d * d;
}
