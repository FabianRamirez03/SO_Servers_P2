#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include "../util/sobel.h"
#include <png.h>
#include <jpeglib.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h> // Incluir la biblioteca de señales
#include <sys/wait.h>
#include <sys/stat.h>


#define MAX_QUEUE_SIZE 100

int front = -1;
int rear =  -1;
int num_elements =  0;

void enqueue(char *value, char queue[MAX_QUEUE_SIZE][100000], sem_t sem_mutex)
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

char *dequeue(sem_t sem_mutex, char queue[MAX_QUEUE_SIZE][100000])
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

int queue_size(char queue[MAX_QUEUE_SIZE][100000])
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


/*void enqueue(char *value, char queue[MAX_QUEUE_SIZE][100000], sem_t sem_mutex)
{
    sem_wait(&sem_mutex);
    if ((*front == *rear + 1) || (*front == 0 && *rear == MAX_QUEUE_SIZE - 1))
    {
        printf("Queue is full\n");
    }
    else
    {
        if (*front == -1)
        {
            front = 0;
        }
        rear = (int *) (*rear + 1) % MAX_QUEUE_SIZE;
        strcpy(queue[*rear], value);
    }
    sem_post(&sem_mutex); // release semaphore
}

char *dequeue(sem_t sem_mutex, char queue[MAX_QUEUE_SIZE][100000])
{
    sem_wait(&sem_mutex);
    if (front == -1 || front > rear)
    {
        sem_post(&sem_mutex);
        return NULL;
    }
    else
    {
        char value = queue[*front];
        if (&front == &rear)
        {
            front = -1;
            rear = -1;
        }
        else
        {
            front = (&front + 1) % MAX_QUEUE_SIZE;
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




// Sacar y eliminar el elemento al inicio de la cola
void dequeue_pre(char queue[MAX_QUEUE_SIZE][100000], int *num_elements)
{
    if (num_elements  == 0) {
        // La cola está vacía, no hay nada que sacar
        return;
    }

    // Sacar el primer elemento de la cola
    char *elem = queue[0];

    // Mover los elementos restantes una posición hacia el inicio
    for (int i = 1; i < *num_elements; i++) {
        strcpy(queue[i-1], queue[i]);
    }

    // Decrementar el contador de elementos en la cola
    *num_elements -= 1;

    // Opcionalmente, limpiar el último elemento para evitar duplicados
    memset(queue[*num_elements], 0, sizeof(queue[*num_elements]));
}


void enqueue_pre(char queue[MAX_QUEUE_SIZE][100000], char* message)
{
    // Si el tamaño de la cola es igual al tamaño máximo, no se puede agregar más elementos
    if (*num_elements == MAX_QUEUE_SIZE)
    {
        printf("La cola está llena. No se puede agregar más elementos.\n");
        return;
    }

    // Agregar el elemento al final de la cola
    strcpy(queue[*rear], message);

    // Actualizar el tamaño y la posición del puntero trasero de la cola
    (*num_elements)++;
    (*rear)++;

    // Si se llega al final del arreglo, regresar al principio
    if (*rear == MAX_QUEUE_SIZE)
    {
        *rear = 0;
    }

    printf("Elemento agregado correctamente.\n");
}*/

