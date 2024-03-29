#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include "../util/tools.h"
#include "../util/img_processing.h"
#include "../util/queue.h"

#define PORT 8081
#define buffer_size 900000
#define MAX_BYTES 1024

char queue[MAX_QUEUE_SIZE][100000];

sem_t sem_mutex, sem_tmpImg, sem_contImg; // semaphore variable

int process_new_request(char *message_received, sem_t sem_tmpImg, sem_t sem_contImg);
int total;
char llave[MAX_BYTES];
void *processing(void *arg);

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
    const char *hello = "Hello from server";
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
        enqueue(buff, queue, sem_mutex);

        // Clear buffer
        memset(buff, 0, sizeof(buff));
    }

    pthread_join(thread_id, NULL);
    return 0;
}

void *processing(void *arg)
{
    clock_t start, end;
    double cpu_time_used;
    int i;
    int restart = 0;
    long double cpu_anterior = 0;

    FILE *csv_file = fopen("GUI/data/FIFO_single.csv", "a");
    if (csv_file == NULL)
    {
        printf("Error al abrir el archivo\n");
        return NULL;
    }
    while (1)
    {
        char *message = dequeue(sem_mutex, queue);
        if (message != NULL)
        {
            i++;
            start = clock();
            // printf("Processing message: %s\n", message);
            printf("Processing msg...");
            process_new_request(message, sem_tmpImg, sem_contImg);
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            fprintf(csv_file, "%f, %s\n", cpu_time_used, llave);
            printf("%d: time %f Key: %s\n", i, cpu_time_used, llave);
        }
        if (i == total && total != 0)
        {
            fclose(csv_file);
            color("Cyan");
            printf("Se han procesado todos los mensajes\n");


            // CSV
            FILE *csv_file = fopen("GUI/data/FIFO.csv", "a");
            if (csv_file == NULL)
            {
                printf("Error al abrir el archivo\n");
                return NULL;
            }

            struct rusage usage;

            // Obtener el uso de recursos del proceso actual
            if (getrusage(RUSAGE_SELF, &usage) != 0)
            {
                printf("Error al obtener el uso de recursos\n");
                return NULL;
            }

            // Imprimir la cantidad de memoria utilizada en KB
            printf("Memoria utilizada: %ld KB\n", usage.ru_maxrss);

            // Imprimir el tiempo de CPU utilizado en segundos y guardarlo en una variable
            long double tiempo_cpu = (long double)usage.ru_stime.tv_sec + ((long double)usage.ru_stime.tv_usec / 1000000.0) - cpu_anterior;
            printf("Tiempo de CPU utilizado: %Lfsegundos\n", tiempo_cpu);
            printf("Tiempo de CPU del sistema utilizado: %ld.%06ld segundos\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
            cpu_anterior = (long double)usage.ru_stime.tv_sec + ((long double)usage.ru_stime.tv_usec / 1000000.0);


            char memory[20];

            sprintf(memory, "%ld", usage.ru_maxrss);

            fprintf(csv_file, "%s, %Lf, %d\n", memory, tiempo_cpu, total);
            fclose(csv_file);

            restart = 1;
            total = 0;

        }
        if (restart == 1)
        {
            printf("Reiniciando...\n");
            i = 0;
            restart = 0;
            csv_file = fopen("GUI/data/FIFO_single.csv", "a");
            if (csv_file == NULL)
            {
                printf("Error al abrir el archivo\n");
                return NULL;
            }
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
        printf("Error: %s\n", error.text); // Imprimir el error en caso de que ocurra
        color("Blanco");
        return 1;
    }

    const char *nombre = json_string_value(json_object_get(json_obj, "nombre")); // Obtener la cadena con clave "nombre"
    const char *key = json_string_value(json_object_get(json_obj, "key"));       // Obtener la cadena con clave "nombre"
    total = json_integer_value(json_object_get(json_obj, "total"));              // Obtener el entero con clave "edad"
    const char *base64_string = json_string_value(json_object_get(json_obj, "data"));

    printf("Nombre: %s\n", nombre); // Imprimir la cadena con clave "nombre"
    printf("Key: %s\n", key);       // Imprimir la cadena con clave "key"
    printf("total: %d\n", total);   // Imprimir el entero con clave "total"

    const char *path = "Servers/FIFO_db/";

    // compare key with llave
    if (strcmp(key, llave) != 0)
    {
        // llave = key;
        strcpy(llave, key);
    }

    sem_wait(&sem_tmpImg);
    base64_to_image(base64_string, key, path); // Modifico la imagen temporal
    sem_post(&sem_tmpImg);

    sobel_filter(nombre, path, sem_contImg, key);

    json_decref(json_obj); // Liberar la memoria utilizada por el objeto JSON

    return 0;
}
