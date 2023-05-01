#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sobel.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

void sobel_filter(const char *nombre, const char *path, sem_t *semaphore)
{
    cv::Mat imagen = cv::imread(nombre, cv::IMREAD_GRAYSCALE);
    if (imagen.empty())
    {
        std::cout << "Error al cargar la imagen" << std::endl;
        return;
    }

    sem_wait(semaphore);

    static int contador = 0;
    std::string nombre_con_contador = std::to_string(contador++) + "_" + std::string(nombre);

    std::string path_con_nombre = std::string(path) + "/" + nombre_con_contador;

    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        if (errno == ENOENT)
        {
            if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
            {
                std::cout << "Error al crear el directorio" << std::endl;
                sem_post(semaphore);
                return;
            }
        }
        else
        {
            std::cout << "Error al abrir el directorio" << std::endl;
            sem_post(semaphore);
            return;
        }
    }
    else
    {
        closedir(dir);
    }

    cv::Mat sobel;
    cv::Sobel(imagen, sobel, CV_8U, 1, 1);
    cv::imwrite(path_con_nombre, sobel);

    sem_post(semaphore);
}
