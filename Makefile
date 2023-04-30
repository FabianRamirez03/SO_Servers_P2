all: Client fifo sobel

Client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm -ljansson
	rm client.o

fifo:
	gcc -c Servers/FIFO.c
	gcc FIFO.o -o out/FIFO -ljansson
	rm FIFO.o

sobel:
	g++ -o out/sobel util/sobel.c `pkg-config --cflags --libs opencv4` -std=c++11

libs:
	sudo apt-get install libssl-dev
	sudo apt-get install libjansson-dev