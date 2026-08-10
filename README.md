# open-neural-network

A lightweight neural network library in C that offloads its forward pass to the GPU using raw OpenGL — no CUDA, no compute shaders, no external ML frameworks. Matrix math for each layer is expressed as a vertex shader and executed via **transform feedback**, letting a graphics-only pipeline double as a general-purpose compute backend.

> **Status: Work in progress.** Forward pass and loss computation work; training (backpropagation / gradient descent) is not yet implemented.

## Why OpenGL instead of compute shaders?

Compute shaders (`GL_COMPUTE_SHADER`) are the "normal" way to do GPU compute in OpenGL, but they require OpenGL 4.3+ and aren't available in every context. This project instead repurposes the classic **vertex shader → transform feedback** pipeline — normally used for rendering — to run parallel floating-point computations across GPU cores, then reads the results back to the CPU. Each GPU "draw call" is really a batch of independent per-neuron calculations, with one shader invocation per output neuron.

## Features

- **GPU-accelerated forward pass** — linear layer computation (weighted sum + activation) executed as a vertex shader, dispatched with `glDrawArraysInstanced` (one instance per output neuron)
- **Sigmoid activation**, computed directly in GLSL
- **Transform feedback readback** — results are captured via a transform feedback buffer and copied back to CPU memory with `glGetBufferSubData`, avoiding a full render/rasterize step
- **Tensors as matrix operations** — inputs and weights are packed into a single GPU buffer (SSBO) per layer and unpacked in-shader
- **Adaptive layer sizing** — layers are created at runtime with arbitrary input/output dimensions (`onn_linear`)
- **Mean Squared Error loss**, also computed on the GPU via a dedicated shader pair
- **OpenGL error-checking macro** (`glc`) wrapping every GL call for easier debugging, plus a simple file logger

### In progress
- Gradient descent and model training (backward pass not yet implemented)

## Tech Stack

| Component | Details |
|---|---|
| Language | C |
| Graphics API | OpenGL 4.3 (core profile) |
| Windowing / context | GLFW |
| GL function loading | [glad](https://glad.dav1d.de/) (vendored in `includes/glad`, prebuilt as `libs/libglad.a`) |
| Shader language | GLSL 4.30 |
| Build | GNU Make + GCC |

## How It Works

1. **Context setup** (`gl_init`) — creates a hidden GLFW window and OpenGL 4.3 core-profile context (a window is required to get a valid GL context, even though nothing is actually rendered to screen), initializes glad, and sets up a VAO/VBO.
2. **Shader loading** (`gl_read_shaders`, `gl_compile_shaders`) — reads a vertex/fragment shader pair from disk, compiles and links them into a program, and registers an output variable as a **transform feedback varying** so its value can be captured instead of rasterized.
3. **Layer computation** (`gl_compute`):
   - Packs the layer's input vector and weight matrix into a single byte buffer, alongside a small header (input/output size), and uploads it to the GPU as a Shader Storage Buffer Object (SSBO).
   - Binds a transform feedback buffer sized to the layer's output.
   - Disables rasterization (`GL_RASTERIZER_DISCARD`) since only the transform-feedback output is needed, not any pixels.
   - Runs `glDrawArraysInstanced` with one instance per output neuron — each shader invocation reads the full input vector and its neuron's weight row from the SSBO, computes a weighted sum, applies sigmoid, and writes one output value.
   - Reads the results back into CPU memory with `glGetBufferSubData`.
4. **Loss computation** (`gl_mse`) — same pattern as `gl_compute`, but with a shader pair (`src/shaders/mse/`) that computes per-element squared error, then sums and halves the result on the CPU side.
5. **Network definition** (`onn_linear`) — allocates a `nn_layer` struct holding input/output sizes and a weight matrix; layers are chained together manually in application code (see `src/main.c`) by feeding one layer's output into the next.

## Project Structure

```
open-neural-network/
├── makefile                     # Build targets: open_nn, linux, sanitizer
├── src/
│   ├── main.c                     # Example: builds a 2→3→2 network and runs a
│   │                               # forward pass + MSE loss on hardcoded data
│   ├── prototype.c                # Earlier prototype using uniforms/matrices
│   │                               # instead of SSBOs (kept for reference)
│   ├── open_nn/
│   │   └── open_nn.c                # Core library: GL context setup, shader
│   │                                 # compilation, layer compute, MSE loss
│   └── shaders/
│       ├── index.vs / index.fs        # Forward pass shader (weighted sum + sigmoid)
│       └── mse/
│           ├── mse.vs / mse.fs          # Per-element squared error shader
├── includes/
│   ├── open_nn/open_nn.h            # Public API: structs (gl_instance, nn_layer,
│   │                                 # gl_shader_file) and function declarations
│   ├── glad/                        # Vendored OpenGL loader
│   └── utils/utils.h
├── libs/
│   └── libglad.a                    # Prebuilt glad static library
└── log.txt                        # Runtime log output (lprintf)
```

## Getting Started

### Prerequisites
- GCC
- GLFW3 development headers (`libglfw3-dev` on Debian/Ubuntu)
- OpenGL / GLU development headers (`libgl1-mesa-dev`, `libglu1-mesa-dev`)
- A GPU/driver supporting OpenGL 4.3 core profile

### Build

```bash
git clone https://github.com/robesckey2k6/open-neural-network.git
cd open-neural-network

# Compile the library object file
make open_nn

# Build the example executable (Linux)
make linux
```

This produces a `main` executable.

### Run

```bash
./main
```

Runs the example in `src/main.c`: builds a small 2-input → 3-hidden → 2-output network with hardcoded weights, performs a forward pass through both layers on the GPU, and prints the resulting activations plus the MSE loss against a target output.

### Debug build

```bash
make sanitizer
```

Builds with AddressSanitizer enabled for catching memory errors during development.

## API Overview

```c
gl_instance*    gl_init(void);
gl_shader_file* gl_read_shaders(const char* vertex_file, const char* fragment_file);
int             gl_compile_shaders(gl_instance* instance, gl_shader_file* shader_file, const char* output_varying_name);

nn_layer*       onn_linear(unsigned int input, unsigned int output, float* weights);
float*          gl_compute(gl_instance* instance, nn_layer* layer, float* data);
float           gl_mse(gl_instance* instance, float* result, float* actual, unsigned int count);
```

Every GL call in the library is wrapped in a `glc(...)` macro that clears the GL error queue beforehand and checks/logs it afterward, making shader and buffer errors easier to trace during development.

## License

No license specified — add one if you intend to share or open-source this project.
