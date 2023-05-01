#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <jansson.h>
#include "../util/sobel.h"
#include <png.h>
#include <jpeglib.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <pthread.h>
#include <semaphore.h>
#include "../util/tools.h"

#define PORT 8081
#define buffer_size 900000
#define MAX_BYTES 1024

#define MAX_QUEUE_SIZE 150

char queue[MAX_QUEUE_SIZE][100000];
int front = -1;
int rear = -1;
sem_t sem_mutex, sem_tmpImg, sem_contImg; // semaphore variable

int process_new_request(char *message_received, sem_t sem_tmpImg, sem_t sem_contImg);
void enqueue(char *value);
char *dequeue();
void display();
void *processing(void *arg);
int base64_to_image(const char *base64_string);

int main(int argc, char **argv)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, processing, NULL);

    sem_post(&sem_mutex);
    sem_post(&sem_tmpImg);
    sem_post(&sem_contImg);

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

        // Read size of message first
        int longitud;
        if (recv(new_socket, &longitud, sizeof(longitud), 0) == -1)
        {
            perror("recv");
            exit(1);
        }

        longitud = ntohl(longitud);
        printf("Longitud del mensaje: %d\n", longitud);
        int bytes_recibidos = 0;
        char buff[longitud + 1];

        while (1)
        {
            int bytes_disponibles = recv(new_socket, buff + bytes_recibidos, longitud - bytes_recibidos, 0);
            if (bytes_disponibles == -1)
            {
                // Error al recibir los datos
                color("Rojo");
                printf("Error al recibir los datos\n");
                color("Blanco");
                break;
            }
            else if (bytes_disponibles == 0)
            {
                // El cliente ha cerrado la conexión
                color("Rojo");
                printf("El cliente ha cerrado la conexión\n");
                color("Blanco");
                break;
            }
            else
            {
                bytes_recibidos += bytes_disponibles;
                if (bytes_recibidos == longitud)
                {
                    // Todos los bytes del mensaje han sido recibidos
                    color("Verde");
                    printf("Todos los bytes del mensaje han sido recibidos\n");
                    printf("Bytes recibidos: %d\n", bytes_recibidos);
                    color("Blanco");
                    send(new_socket, "Ok", strlen("Ok"), 0);
                    break;
                }
            }
        }
        buff[longitud] = '\0';
        enqueue(buff);

        // Clear buffer
        memset(buff, 0, sizeof(buff));
    }

    pthread_join(thread_id, NULL);
    return 0;
}

void *processing(void *arg)
{
    while (1)
    {
        char *message = dequeue();
        if (message != NULL)
        {
            // printf("Processing message: %s\n", message);
            printf("Processing msg...");
            process_new_request(message, sem_tmpImg, sem_contImg);
        }
    }
    return 0;
}

int process_new_request(char *message_received, sem_t sem_tmpImg, sem_t sem_contImg)
{
    printf("Incia el procesamiento\n");

    json_error_t error; // Estructura para almacenar errores

    // printf("Mensaje recibido: %s\n", message_received);

    json_t *json_obj = json_loads(message_received, 0, &error); // Deserializar la cadena JSON en un objeto JSON

    if (json_obj == NULL)
    {
        color("Rojo");
        printf( "Error: %s\n", error.text); // Imprimir el error en caso de que ocurra
        color("Blanco");
        return 1;
    }

    const char *nombre = json_string_value(json_object_get(json_obj, "nombre")); // Obtener la cadena con clave "nombre"
    const char *key = json_string_value(json_object_get(json_obj, "key"));       // Obtener la cadena con clave "nombre"
    int total = json_integer_value(json_object_get(json_obj, "total"));          // Obtener el entero con clave "edad"
    const char *base64_string = json_string_value(json_object_get(json_obj, "data"));

    printf("Nombre: %s\n", nombre); // Imprimir la cadena con clave "nombre"
    printf("Key: %s\n", key);       // Imprimir la cadena con clave "key"
    printf("total: %d\n", total);   // Imprimir el entero con clave "total"

    sem_wait(&sem_tmpImg);

    base64_to_image(base64_string); // Modifico la imagen temporal

    sem_post(&sem_tmpImg);

    const char *path = "Servers/FIFO_db/";

    sobel_filter(nombre, path, sem_contImg, key);

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

int queue_size()
// returns the size in bytes of the queue
{
    if (front == -1)
    {
        printf("Queue is empty\n");
        return 0;
    }
    else
    {
        int size = 0;
        int i;
        for (i = front; i != rear; i = (i + 1) % MAX_QUEUE_SIZE)
        {
            size += strlen(queue[i]);
        }
        size += strlen(queue[rear]);
        return size;
    }
}

int base64_to_image(const char *base64_string)
{
    char *path_to_save = "./Servers/FIFO_db/temp.png";

    BIO *bio, *b64;
    FILE *fp;
    int image_size = strlen(base64_string);
    char *buffer = (char *)malloc(image_size);
    memset(buffer, 0, image_size);

    bio = BIO_new_mem_buf((void *)base64_string, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_read(bio, buffer, image_size);

    fp = fopen(path_to_save, "wb");
    if (!fp)
    {
        printf("No se pudo abrir el archivo de imagen para escritura\n");
        BIO_free_all(bio);
        free(buffer);
        return -1;
    }

    fwrite(buffer, 1, image_size, fp);
    fclose(fp);

    BIO_free_all(bio);
    free(buffer);

    return 0;
}