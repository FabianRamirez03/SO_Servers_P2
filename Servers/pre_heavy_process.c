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
#include <sys/mman.h> // para mmap(), munmap()
#include <fcntl.h>    // para O_CREAT, O_RDWR
#include <unistd.h>   // para ftruncate()
#include <stdlib.h>   // para exit()
#include <stdio.h>    // para printf()
#include <string.h>   // para strcmp()
#include <semaphore.h>// para sem_init(), sem_wait(), sem_post()
#include "../util/tools.h"
#include <signal.h> // Incluir la biblioteca de señales
#include <sys/wait.h>
#include <sys/stat.h>
#include "../util/img_processing.h"
#include "../util/queue.h"
#include <sys/shm.h>
#include <sys/resource.h>

#define PORT 8084
#define buffer_size 900000
#define MAX_BYTES 1024

//char queue[MAX_QUEUE_SIZE][100000];
//char(*queue)[100000];
sem_t sem_mutex, sem_tmpImg, sem_contImg; // semaphore variable
sem_t sem_pid, sem_data;                            // semaphore variable

pid_t PARENT_PID;

int process_new_request(char *message_received, sem_t sem_tmpImg, sem_t sem_contImg);

int processes = 0;

int requests_processed = 0;

int done = 1;

void display();
void *processing(void *arg);


#define QUEUE_SIZE 100 // tamaño máximo de la cola

typedef struct {
    char messages[QUEUE_SIZE][1000000];
    int head;
    int tail;
} queue_t;

char *dequeue_pre(queue_t *queue) ;
void enqueue_pre(queue_t *queue, const char *message);
int save_data(double cpu_time_used, int total);
queue_t *queue;
char llave[MAX_BYTES];
long double cpu_anterior = 0;

int main(int argc, char **argv)
{
    key_t key = 1234;
    int shmid;
    //char(*queue)[100000];

    //get -p from command line and save it ina a a variable
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            processes = atoi(argv[i + 1]);
        }
    }
    
    

    sem_init(&sem_pid, 0, 1); // Inicializa el semáforo en 1

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, processing, NULL);

    sem_post(&sem_mutex);
    sem_post(&sem_tmpImg);
    sem_post(&sem_contImg);
    sem_post(&sem_pid);
    sem_post(&sem_data);

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
        enqueue_pre(queue, buff);

        // Clear buffer and message received
        memset(buff, 0, sizeof(buff));
    }

    pthread_join(thread_id, NULL);
    return 0;
}
void *processing(void *arg)
{
    FILE *csv_file = fopen("GUI/data/FIFO_single.csv", "a");
    if (csv_file == NULL)
    {
        printf("Error al abrir el archivo\n");
        return NULL;
    }

    // Crear la memoria compartida para el queue
    int fd = shm_open("/myqueue", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(fd, sizeof(queue_t)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    queue = (queue_t*) mmap(NULL, sizeof(queue_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (queue == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    close(fd);

    // Inicializar el semáforo para el queue
    sem_t *sem_queue;
    sem_queue = sem_open("/mysemaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (sem_queue == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Obtener el ID del proceso padre al inicio de la ejecución
    pid_t parent_pid;
    sem_t *sem_pid;
    sem_pid = sem_open("/mypid", O_CREAT, S_IRUSR | S_IWUSR, 1);
    sem_wait(sem_pid);
    parent_pid = getpid();
    sem_post(sem_pid);

    // Crear los 6 procesos hijos
    pid_t pid[6];
    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            perror("fork");
            exit(1);
        } else if (pid[i] == 0) {
            // Proceso hijo
            while (1) {
                // Leer un mensaje del queue
                sem_wait(sem_queue);
                if (queue->head != queue->tail) {
                    //char *message = queue->messages[queue->tail];
                    //queue->tail = (queue->tail + 1) % QUEUE_SIZE;
                    char * message = dequeue_pre(queue);
                    
                    sem_post(sem_queue);

                    printf("Proceso hijo %d procesando mensaje...\n", i);
                    // Procesar el mensaje
                    process_new_request(message, sem_tmpImg, sem_contImg);
                } else {
                    sem_post(sem_queue);
                    // Esperar un poco antes de volver a leer el queue
                    usleep(100000);
                }
            }
            exit(0);
        }
    }
    
    // código del proceso padre
    while (1)
    {
        // espera a que haya un mensaje en la cola
        sem_wait(&sem_mutex);
        sem_wait(&sem_data);
        char *message =  dequeue_pre(queue);
        sem_post(&sem_mutex);

        if (message != NULL)
        {
            printf("Enviando mensaje a hijo...\n");

            // enviar el mensaje a un proceso hijo libre
            int free_child = -1;
            for (int i = 0; i < 6; i++)
            {
                if (waitpid(pid[i], NULL, WNOHANG) == pid[i])
                {
                    free_child = i;
                    break;
                }
            }

            if (free_child >= 0)
            {
                sem_wait(sem_pid);
                printf("Enviando mensaje a hijo %d...\n", free_child);
                sem_post(sem_pid);

                sem_wait(&sem_mutex);
                dequeue_pre(queue); // sacar el mensaje de la cola
                sem_post(&sem_mutex);

                // enviar el mensaje al proceso hijo libre
                sem_post(&sem_data); // desbloquear al proceso hijo
            }
            else
            {
                printf("Todos los hijos están ocupados, esperando...\n");
            }
        }
    }

}

int process_new_request(char *message_received, sem_t sem_tmpImg, sem_t sem_contImg)
{
    printf("Incia el procesamiento\n");
    clock_t start, end;
	double cpu_time_used;

	start = clock();

    json_error_t error; // Estructura para almacenar errores

    // printf("Mensaje recibido: %s\n", message_received);

    json_t *json_obj = json_loads(message_received, 0, &error); // Deserializar la cadena JSON en un objeto JSON

    if (json_obj == NULL)
    {
        color("Rojo");
        fprintf(stderr, "Error: %s\n", error.text); // Imprimir el error en caso de que ocurra
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

    // Guardar el string en un archivo de texto
    const char *path = "Servers/pre_heavy_db/";
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

    end = clock();
	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    sem_wait(&sem_data);
	save_data(cpu_time_used, total);
	sem_post(&sem_data);

    return 0;
}


char *dequeue_pre(queue_t *queue) {
    // Si la cola está vacía, devolver NULL
    if (queue->head == queue->tail) {
        return NULL;
    }
    // Obtener el mensaje en la posición de la cabeza de la cola
    char *message = queue->messages[queue->head];
    // Actualizar la posición de la cabeza de la cola
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    // Devolver el mensaje
    return message;
}

void enqueue_pre(queue_t *queue, const char *message) {
    if ((queue->tail + 1) % QUEUE_SIZE == queue->head) {
        // La cola está llena
        printf("La cola está llena, no se puede agregar el mensaje: %s\n", message);
        return;
    }

    // Copiar el mensaje a la cola
    strcpy(queue->messages[queue->tail], message);

    // Actualizar la posición del tail
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;

    //printf("El mensaje %s fue agregado a la cola\n", message);
}

int save_data(double cpu_time_used, int total){
	requests_processed ++;
	FILE *csv_file = fopen("GUI/data/Pre_Heavy_single.csv", "a");
	if (csv_file == NULL)
    {
        printf("Error al abrir el archivo\n");
        return 1;
    }
	fprintf(csv_file, "%f,%s\n", cpu_time_used, llave);
	fclose(csv_file);
    printf("Requests processed: %d\n", requests_processed);
	if (requests_processed ==  total || requests_processed * processes >= total * 0.8 && done == 1){
		printf("Guarda datos finales\n");
		FILE *csv_file = fopen("GUI/data/Pre_Heavy.csv", "a");
		if (csv_file == NULL)
		{
		    printf("Error al abrir el archivo\n");
		    return 1;
		}

		color("Cyan");
        printf("Se han procesado todos los mensajes\n");

		struct rusage usage;

		// Obtener el uso de recursos del proceso actual
		if (getrusage(RUSAGE_SELF, &usage) != 0)
		{
			printf("Error al obtener el uso de recursos\n");
			return 1;
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

		fprintf(csv_file, "%s,%Lf,%d\n", memory, tiempo_cpu, requests_processed);
		fclose(csv_file);
        if(requests_processed >= total){
            requests_processed = 0;
            done = 1;
        }
        else{
            done = 0;
        }
		return 0;

	}
	return 0;

}

