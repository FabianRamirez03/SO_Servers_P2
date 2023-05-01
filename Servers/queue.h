#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
sem_t sem_mutex; // semaphore variable



// Definición de la estructura del nodo de la queue
typedef struct Node {
    char * value;
    struct Node *next;
} Node;

// Definición de la estructura de la queue
typedef struct Queue {
    Node *head;
    Node *tail;
    int size;
} Queue;

// Función para agregar un nodo a la queue
void enqueue(Queue *q, char* value) {
    sem_wait(&sem_mutex);
    Node *new_node = (Node *) malloc(sizeof(Node));
    new_node->value = value;

    if (q->size == 0) {
        q->head = new_node;
    } else {
        q->tail->next = new_node;
    }

    new_node->next = q->head;
    q->tail = new_node;
    q->size++;
    sem_post(&sem_mutex);
}

// Función para obtener el siguiente nodo de la queue
char*  dequeue(Queue *q) {
    sem_wait(&sem_mutex);
    if (q->size == 0) {
        return NULL;
    }

    Node *node_to_remove = q->head;
    char* value = node_to_remove->value;

    if (q->size == 1) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = node_to_remove->next;
        q->tail->next = q->head;
    }

    free(node_to_remove);
    q->size--;
    sem_post(&sem_mutex);
    return value;
}

// Función para eliminar todo el contenido de la queue
void clear_queue(Queue *q) {
    Node *current_node = q->head;
    Node *next_node;

    for (int i = 0; i < q->size; i++) {
        next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}