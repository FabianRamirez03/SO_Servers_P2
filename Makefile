client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm
	rm client.o