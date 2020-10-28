all: example.o bmp.o images.o
	gcc example.o bmp.o images.o -o example.exe

example.o:
	gcc -c example.c -o example.o

bmp.o:
	gcc -c ../bmp.c -o bmp.o

images.o:
	gcc -c ../images.c -o images.o