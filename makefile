

open_nn:
	gcc src/open_nn/open_nn.c -Iincludes -c open_nn.o
linux:
	gcc  src/main.c open_nn.o -Iincludes -Llibs -lglad -lglfw -lGL -lm -ldl -lGLU -o main
sanitizer:
	gcc -g -fsanitize=address -fno-omit-frame-pointer src/main.c open_nn.o -Iincludes -Llibs -lglad -lglfw -lGL -lm -ldl -lGLU -o main
