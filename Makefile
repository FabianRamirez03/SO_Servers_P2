all: Client fifo

Client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm -ljansson -pthread
	rm client.o

fifo:
	gcc -c Servers/FIFO.c
	gcc FIFO.o -o out/FIFO -ljansson  -pthread 
	rm FIFO.o

sobel:
	g++ -o out/sobel util/sobel.c `pkg-config --cflags --libs opencv4` -std=c++11
