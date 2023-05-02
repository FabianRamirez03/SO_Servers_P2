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
int rear = -1;

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