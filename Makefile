all: Client fifo 

Client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm -ljansson -pthread
	rm client.o

fifo:
	g++ -o out/FIFO Servers/FIFO.c util/sobel.c `pkg-config --cflags --libs opencv4 jansson` -lssl -lcrypto -std=c++11


libs:
	sudo apt-get install libssl-dev
	sudo apt-get install libjansson-dev