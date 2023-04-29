all: Client fifo

Client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm -ljansson
	rm client.o

fifo:
	gcc -c Servers/FIFO.c
	gcc FIFO.o -o out/FIFO -ljansson
	rm FIFO.o

