all: build_Client build_heavy_process build_fifo threads

build_Client:
	gcc -c Clients/client.c
	gcc client.o -o out/client -lssl -lcrypto -lm -ljansson -pthread
	rm client.o

build_heavy_process:
	g++ -o out/heavy_process Servers/heavy_process.c util/sobel.c `pkg-config --cflags --libs opencv4 jansson` -lssl -lcrypto -std=c++11 -pthread

threads:
	g++ -o out/Threads Servers/Threads.c util/sobel.c `pkg-config --cflags --libs opencv4 jansson` -lssl -lcrypto -pthread -std=c++11

build_fifo:
	g++ -o out/FIFO Servers/FIFO.c util/sobel.c `pkg-config --cflags --libs opencv4 jansson` -lssl -lcrypto -std=c++11 -pthread

install_libs:
	sudo apt-get install libssl-dev
	sudo apt-get install libjansson-dev
	sudo apt-get install libjpeg-dev

run_client:
	./out/client -p 8081 -t 2 -c 5 -i ./images/arbol.png -ip 218.122.255 -lz

run_fifo:
	rm -rf ./Servers/FIFO_db/*
	mkdir ./Servers/FIFO_db/tmp/
	./out/FIFO

run_heavy_process:
	rm -rf ./Servers/heavy_db/*
	mkdir ./Servers/heavy_db/tmp/
	./out/heavy_process