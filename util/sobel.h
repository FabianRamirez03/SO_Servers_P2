#ifndef SOBEL_H
#define SOBEL_H

#include <stdlib.h>
#include <semaphore.h>

extern void sobel_filter(const char *nombre, const char *path, sem_t semaphore, const char *client_id);

#endif /* SOBEL_H */
