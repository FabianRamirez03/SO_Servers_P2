#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <jansson.h>
#include <pthread.h>
#include <semaphore.h>

#define PORT 8081
#define buffer_size 1000000

#define MAX_QUEUE_SIZE 10

char queue[MAX_QUEUE_SIZE][buffer_size];
int front = -1;
int rear = -1;
sem_t sem_mutex; // semaphore variable

int process_new_request(const char *message_received);
void enqueue(char *value);
char *dequeue();
void display();
void *listen_sock();

int main(int argc, char **argv)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, listen_sock, NULL);
    sem_post(&sem_mutex);

    while (1)
    {
        char *message = dequeue();
        if (message != NULL)
        {
            printf("Processing message: %s\n", message);
            process_new_request(message);
            display();
        }
    }

    pthread_join(thread_id, NULL);
    return 0;
}

void *listen_sock()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[buffer_size] = {0};
    char message_rec[buffer_size] = {0};
    message_rec[sizeof(message_rec) - 1] = '\0';
    char *hello = "Hello from server";
    int total_bytes_processed = 0;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Waiting for new connection...\n");

        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection established.\n");
        // Read message from client
        valread = read(new_socket, buffer, buffer_size);
        printf("Received %d bytes from client.\n", valread);
        total_bytes_processed = total_bytes_processed + valread;
        // Check if client closed connection
        if (valread == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        // Process received data
        // printf("Processing data: %s\n", buffer);
        strcat(message_rec, buffer);

        if (buffer[valread - 1] == '*' && buffer[valread - 2] == '*' && buffer[valread - 3] == '*')
        {
            printf("End of message received.\n");
            // Remove last 3 characters
            message_rec[strlen(message_rec) - 3] = '\0';

            buffer[strlen(buffer) - 3] = '\0';

            // process_new_request(message_rec);

            enqueue(message_rec);

            // Clear buffer and message received
            memset(buffer, 0, buffer_size);
            memset(message_rec, 0, buffer_size);
            total_bytes_processed = 0;
            valread = 0;

            // Send message to client
            send(new_socket, hello, strlen(hello), 0);
            printf("Hello message sent\n");
        }
    }

    return 0;
}

int process_new_request(const char *message_received)
{
    printf("Incia el procesamiento\n");

    json_error_t error; // Estructura para almacenar errores

    printf("Mensaje recibido: %s\n", message_received);

    json_t *json_obj = json_loads(message_received, 0, &error); // Deserializar la cadena JSON en un objeto JSON

    if (json_obj == NULL)
    {
        fprintf(stderr, "Error: %s\n", error.text); // Imprimir el error en caso de que ocurra
        return 1;
    }

    const char *nombre = json_string_value(json_object_get(json_obj, "nombre")); // Obtener la cadena con clave "nombre"
    const char *key = json_string_value(json_object_get(json_obj, "key"));       // Obtener la cadena con clave "nombre"
    int total = json_integer_value(json_object_get(json_obj, "total"));          // Obtener el entero con clave "edad"
    const char *base64_string = json_string_value(json_object_get(json_obj, "data"));

    printf("Nombre: %s\n", nombre); // Imprimir la cadena con clave "nombre"
    printf("Key: %s\n", key);       // Imprimir la cadena con clave "nombre"
    printf("total: %d\n", total);   // Imprimir el entero con clave "edad"

    // Guardar el string en un archivo de texto

    FILE *out = fopen("procesado.txt", "w");
    if (!out)
    {
        printf("No se pudo abrir el archivo de salida\n");
        return 1;
    }
    fprintf(out, "%s", base64_string);
    fclose(out);

    json_decref(json_obj); // Liberar la memoria utilizada por el objeto JSON

    return 0;
}

void enqueue(char *value)
{
    sem_wait(&sem_mutex);
    if ((front == rear + 1) || (front == 0 && rear == MAX_QUEUE_SIZE - 1))
    {
        printf("Queue is full\n");
    }
    else
    {
        if (front == -1)
        {
            front = 0;
        }
        rear = (rear + 1) % MAX_QUEUE_SIZE;
        strcpy(queue[rear], value);
    }
    sem_post(&sem_mutex); // release semaphore
}

char *dequeue()
{
    sem_wait(&sem_mutex);
    if (front == -1 || front > rear)
    {
        sem_post(&sem_mutex);
        return NULL;
    }
    else
    {
        char *value = queue[front];
        if (front == rear)
        {
            front = -1;
            rear = -1;
        }
        else
        {
            front = (front + 1) % MAX_QUEUE_SIZE;
        }
        sem_post(&sem_mutex);
        return value;
    }
}

void display()
{
    if (front == -1)
    {
        printf("Queue is empty\n");
    }
    else
    {
        // print size and front and rear
        printf("Queue size: %d\n", rear + 1);
        printf("Front index: %d\n", front);
        printf("Rear index: %d\n", rear);
    }
}