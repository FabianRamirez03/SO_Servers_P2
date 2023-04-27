#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <png.h>
#include <jpeglib.h>

#define PORT 8081

// Inicializar funciones
int is_grayscale_png(char *image_path);
int is_grayscale_jpg(char *image_path);
int is_valid_image(char *image_path);

int main(int argc, char *argv[]) {

	// Inicializa las variables
	int port = -1;
	int threads = -1;
	int cycles = -1;

    char image_path[100] = "";
    char ip[50] = "";

	    // Obtiene el valor de los argumentos pasados en la inicializacion del programa
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            strcpy(image_path, argv[i + 1]);
		}
		if (strcmp(argv[i], "-ip") == 0) {
            strcpy(ip, argv[i + 1]);
        }
        if (strcmp(argv[i], "-p") == 0) { 
            port = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-t") == 0) { 
            threads = atoi(argv[i + 1]);
        }
		if (strcmp(argv[i], "-c") == 0) { 
            cycles = atoi(argv[i + 1]);
        }
    }
    // Se asegura que los parametros fueron correctamente proporcionados y cambiados, sino falla
    if (port == -1 || threads == -1 || cycles == -1 || (strcmp(image_path, "") == 0)  || (strcmp(ip, "") == 0) ) {
        printf("Falta alguno de los argumentos requeridos (-p, -t, -c, -i, -ip)\n");
        return 1;
    }

	if (is_valid_image(image_path) == 1){
		printf("Imagen válida\n");
	}


    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Set server address and port
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send message to server
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Read message from server
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);

    // Close socket
    close(sock);

    return 0;
}

int is_grayscale_png(char* image_path) {
    FILE* fp = fopen(image_path, "rb");
    if (!fp) {
        printf("No se pudo abrir la imagen: %s\n", image_path);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("No se pudo crear la estructura png_ptr para la imagen: %s\n", image_path);
        fclose(fp);
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("No se pudo crear la estructura info_ptr para la imagen: %s\n", image_path);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return 0;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);

    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		printf("Imagen válida\n");
        return 1;
    }

    return 0;
}

int is_grayscale_jpg(char* image_path) {
    FILE* fp = fopen(image_path, "rb");
    if (!fp) {
        printf("No se pudo abrir la imagen: %s\n", image_path);
        return 0;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);

    fclose(fp);

    if (cinfo.num_components == 1) {
		printf("Imagen válida\n");
        jpeg_destroy_decompress(&cinfo);
        return 1;
    }

    jpeg_destroy_decompress(&cinfo);
    return 0;
}


int is_valid_image(char* image_path) {
    char* extension = strrchr(image_path, '.');
    if (extension == NULL) {
        printf("La imagen no tiene una extensión válida: %s\n", image_path);
        return 0;
    }

    extension++; // avanza el puntero para obtener la extensión sin el punto

    if (strcasecmp(extension, "png") == 0) {
        return is_grayscale_png(image_path);
    }
    else if (strcasecmp(extension, "jpg") == 0 || strcasecmp(extension, "jpeg") == 0) {
        return is_grayscale_jpg(image_path);
    }
}