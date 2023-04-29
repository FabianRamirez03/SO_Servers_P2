#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cv::Mat imagen = cv::imread("images/abigail.jpg", cv::IMREAD_GRAYSCALE);
    if (imagen.empty()) {
        std::cout << "Error al cargar la imagen" << std::endl;
        return -1;
    }
    cv::Mat sobel;
    cv::Sobel(imagen, sobel, CV_8U, 1, 1);
    cv::imwrite("images/sobel/fifo/abigail_sobel.jpg", sobel);
    return 0;
}