client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm
	rm client.o

fifo:
	gcc -c Servers/FIFO.c
	gcc FIFO.o -o out/FIFO
	rm FIFO.o

all: client fifo