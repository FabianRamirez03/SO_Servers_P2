#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sobel.h"

void sobel_filter(const char *base64_image)
{
    // Convertir la imagen en base64 a cv::Mat
    std::string image_data = base64_image;
    std::vector<char> buffer(image_data.begin(), image_data.end());
    cv::Mat img = cv::imdecode(buffer, cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        printf("Error al cargar la imagen\n");
        return;
    }

    // Aplicar el filtro de Sobel a la imagen
    cv::Mat sobel;
    cv::Sobel(img, sobel, CV_8U, 1, 1);

    // Guardar la imagen resultante en disco
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);
    std::string filename = "images/sobel/fifo/abigail_sobel.jpg";
    cv::imwrite(filename, sobel, compression_params);
}