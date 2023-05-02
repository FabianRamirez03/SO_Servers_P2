#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sobel.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <semaphore.h>

int save_sobel_result(cv::Mat sobel, const char *path, const char *client_id, const char *nombre);

void sobel_filter(const char *nombre, const char *path, sem_t semaphore, const char *client_id)
{
    /*char temp_path[50];
    strcpy(temp_path, path);
    strcat(temp_path, "temp.png");*/

    char result_name[256];
    snprintf(result_name, sizeof(result_name), "%stmp/tmp_%s.png", path, client_id);
    const char *temp_path = result_name;

    cv::Mat imagen = cv::imread(temp_path, cv::IMREAD_GRAYSCALE);
    if (imagen.empty())
    {
        std::cout << "Error al cargar la imagen" << std::endl;
        return;
    }
    cv::Mat sobel;
    cv::Sobel(imagen, sobel, CV_8U, 1, 1);

    sem_wait(&semaphore);
    save_sobel_result(sobel, path, client_id, nombre);
    sem_post(&semaphore);
}

char *create_client_directory(const char *path, const char *client_ID)
{
    char new_directory_path[100];
    strcpy(new_directory_path, path);
    strcat(new_directory_path, client_ID);
    DIR *dir = opendir(new_directory_path);
    char *result;
    if (dir)
    {
        closedir(dir);

        result = (char *)malloc(strlen(new_directory_path) + 1);
        strcpy(result, new_directory_path);
        return result;
    }
    else if (mkdir(new_directory_path, 0777) == -1)
    {
        printf("Error al crear el directorio.\n");
    }
    else
    {
        printf("El directorio se creó correctamente.\n");
        char *result = (char *)malloc(strlen(new_directory_path) + 1);
        strcpy(result, new_directory_path);
        return result;
    }
    return result;
}

int save_sobel_result(cv::Mat sobel, const char *path, const char *client_id, const char *nombre)
{
    char *path_client_folder = create_client_directory(path, client_id);
    if (strcmp(path_client_folder, "") == 0)
    {
        printf("Fallo al crear el directorio.\n");
        return 1;
    }
    else
    {
        // Obtiene la cantidad de archivos en el directorio
        int file_count = 0;
        DIR *dir;
        struct dirent *entry;
        dir = opendir(path_client_folder);
        if (dir == NULL)
        {
            perror("opendir");
            return -1;
        }
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
            {
                file_count++;
            }
        }
        closedir(dir);

        // Procede a guardar el archivo
        if (file_count < 100)
        {
            char result_name[100];
            snprintf(result_name, sizeof(result_name), "%s/%d_%s", path_client_folder, file_count, nombre);
            cv::imwrite(result_name, sobel);
        }
        else
        {
            printf("Se llegó al limite de los 100 archivos.\n");
        }
    }
    return 0;
}
