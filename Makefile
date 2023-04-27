client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lpng -ljpeg -lm
	rm client.o