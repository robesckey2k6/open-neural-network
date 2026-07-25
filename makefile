

open_nn:
	gcc src/open_nn/open_nn.c -Iincludes -c open_nn.o
linux:
	gcc src/main.c open_nn.o -Iincludes -Llibs -lglad -lglfw -lGL -lm -ldl -o main
