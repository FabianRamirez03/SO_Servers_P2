#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sobel.h"

void sobel_filter(const char *nombre, const char *path)
{
    cv::Mat imagen = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (imagen.empty())
    {
        std::cout << "Error al cargar la imagen" << std::endl;
    }
    cv::Mat sobel;
    cv::Sobel(imagen, sobel, CV_8U, 1, 1);
    cv::imwrite("Servers/FIFO_db/sobel.png", sobel);
}