#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081

int main(int argc, char *argv[]) {

	// Inicializa las variables
	int port = -1;
	int threads = -1;
	int cycles = -1;

    char image_path[100] = "";
    char ip[50] = "";

	    // Obtiene el valor de los argumentos pasados en la inicializacion del programa
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            strcpy(image_path, argv[i + 1]);
		}
		if (strcmp(argv[i], "-ip") == 0) {
            strcpy(ip, argv[i + 1]);
        }
        if (strcmp(argv[i], "-p") == 0) { 
            port = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-t") == 0) { 
            threads = atoi(argv[i + 1]);
        }
		if (strcmp(argv[i], "-c") == 0) { 
            cycles = atoi(argv[i + 1]);
        }
    }
    // Se asegura que los parametros fueron correctamente proporcionados y cambiados, sino falla
    if (port == -1 || threads == -1 || cycles == -1 || (strcmp(image_path, "") == 0)  || (strcmp(ip, "") == 0) ) {
        printf("Falta alguno de los argumentos requeridos (-p, -t, -c, -i, -ip)\n");
        return 1;
    }


    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Set server address and port
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send message to server
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Read message from server
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);

    // Close socket
    close(sock);

    return 0;
}