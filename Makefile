client:
	gcc -c Clients/client.c
	gcc client.o -o out/client
	rm client.o