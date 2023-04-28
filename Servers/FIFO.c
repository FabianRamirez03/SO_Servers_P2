#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <jansson.h>

#define PORT 8081
#define buffer_size 1000000

int process_new_request(const char* message_received, size_t message_length);

int main(int argc, char **argv) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[buffer_size] = {0};
    char *hello = "Hello from server";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for new connection...\n");

        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection established.\n");

        while (1) {
            // Read message from client
            valread = read(new_socket, buffer, buffer_size);
            printf("Received %d bytes from client.\n", valread);

            // Check if client closed connection
            if (valread == 0) {
                printf("Client disconnected.\n");
                break;
            }

            // Process received data
            printf("Processing data: %s\n", buffer);

			process_new_request(buffer, sizeof(buffer));

            // Send message to client
            send(new_socket, hello, strlen(hello), 0);
            printf("Hello message sent\n");

            // Clear buffer
            memset(buffer, 0, sizeof(buffer));
        }

        // Close current socket
        close(new_socket);
    }

    // Close server socket
    close(server_fd);

    return 0;
}

int process_new_request(const char* message_received, size_t message_length){

	json_error_t error;  // Estructura para almacenar errores
    
    json_t *json_obj = json_loadb(message_received, message_length, 0, &error);  // Deserializar la cadena JSON en un objeto JSON
    
    if (json_obj == NULL) {
        fprintf(stderr, "Error: %s\n", error.text);  // Imprimir el error en caso de que ocurra
        return 1;
    }
    
    const char *nombre = json_string_value(json_object_get(json_obj, "nombre"));  // Obtener la cadena con clave "nombre"
	const char *key = json_string_value(json_object_get(json_obj, "key"));  // Obtener la cadena con clave "nombre"
    int total = json_integer_value(json_object_get(json_obj, "total"));  // Obtener el entero con clave "edad"
    
    printf("Nombre: %s\n", nombre);  // Imprimir la cadena con clave "nombre"
	printf("Key: %s\n", key);  // Imprimir la cadena con clave "nombre"
    printf("total: %d\n", total);  // Imprimir el entero con clave "edad"
    
    json_decref(json_obj);  // Liberar la memoria utilizada por el objeto JSON
    
    return 0;
}