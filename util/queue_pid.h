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


#define MAX_QUEUE_SIZE_PID 1000

int front_pid = -1;
int rear_pid = -1;

void enqueue_pid(pid_t value, pid_t * queue, sem_t sem_mutex)
{
    sem_wait(&sem_mutex);
    if ((front_pid == rear_pid + 1) || (front_pid == 0 && rear_pid == MAX_QUEUE_SIZE_PID - 1))
    {
        printf("Queue is full\n");
    }
    else
    {
        if (front_pid == -1)
        {
            front_pid = 0;
        }
        rear_pid = (rear_pid + 1) % MAX_QUEUE_SIZE_PID;
        queue[rear_pid] = value;
    }
    sem_post(&sem_mutex); // release semaphore
}

pid_t dequeue_pid(sem_t sem_mutex, int* queue)
{
    sem_wait(&sem_mutex);
    if (front_pid == -1 || front_pid > rear_pid)
    {
        sem_post(&sem_mutex);
        return NULL;
    }
    else
    {
        int value = queue[front_pid];
        if (front_pid == rear_pid)
        {
            front_pid = -1;
            rear_pid = -1;
        }
        else
        {
            front_pid = (front_pid + 1) % MAX_QUEUE_SIZE_PID;
        }

        sem_post(&sem_mutex);
        return value;
    }
}

void display_pid()
{
    if (front_pid == -1)
    {
        printf("Queue is empty\n");
    }
    else
    {
        // print size and front_pid and rear_pid
        printf("Queue size: %d\n", rear_pid + 1);
        printf("Front index: %d\n", front_pid);
        printf("Rear index: %d\n", rear_pid);
    }
}

/*int queue_size_pid(int queue [MAX_QUEUE_SIZE_PID])
// returns the size in bytes of the queue
{
    if (front_pid == -1)
    {
        printf("Queue is empty\n");
        return 0;
    }
    else
    {
        int size = 0;
        int i;
        for (i = front_pid; i != rear_pid; i = (i + 1) % MAX_QUEUE_SIZE_PID)
        {
            size += strlen(queue[i]);
        }
        size += strlen(queue[rear_pid]);
        return size;
    }
}*/