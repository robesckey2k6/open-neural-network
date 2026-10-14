#version 430 core

layout(std430, binding = 1) readonly buffer HiddenBuf { float hidden_a[]; };
layout(std430, binding = 2) readonly buffer OutBuf    { float out_a[];    };
layout(std430, binding = 3) readonly buffer ExpBuf    { float expected[]; };
layout(std430, binding = 4) readonly buffer WeightBuf { float weights[];  };
layout(std430, binding = 5) readonly buffer LrBuf     { float learning_rate; };

out float outdat;

void main() {
	int k        = gl_InstanceID;
	int n_hidden = hidden_a.length();
	int o        = k / n_hidden;
	int h        = k % n_hidden;

	float a     = out_a[o];
	float delta = (a - expected[o]) * a * (1.0 - a);

	outdat = weights[k] - learning_rate * delta * hidden_a[h];
}
