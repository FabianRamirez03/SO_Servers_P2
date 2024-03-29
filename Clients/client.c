#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <jansson.h>
#include <pthread.h>
#include <semaphore.h>


#define KEY_LENGTH 32
#define buffer_size 1024
#define MAX_BYTES 1024
// Inicializar funciones
int is_valid_image_path(char *image_path);
char* image_to_base64(char* image_path);
char* generate_key(void);
char* get_filename(char* path);

sem_t semaphore;

struct thread_args {
    char* json_str;
    int cycles;
    int port;
	char * ip;
};

int new_connection(char * json_str, int port, char * ip){
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[buffer_size] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Set server address and port
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    int json_len = strlen(json_str);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    int longitud = htonl(json_len);
    printf("longitud: %d\n", ntohl(longitud));

    send(sock, &longitud, sizeof(longitud), 0);

    int bytes_enviados = 0;

    while (bytes_enviados < json_len) {
        int bytes_a_enviar = json_len - bytes_enviados;
        if (bytes_a_enviar > MAX_BYTES) {
            bytes_a_enviar = MAX_BYTES;
        }
        int resultado = send(sock, json_str + bytes_enviados, bytes_a_enviar, 0);
        if (resultado == -1) {
            // Error al enviar los datos
            break;
        }
        bytes_enviados += resultado;
    }

    printf("Hello message sent\n");



    // Read message from server
    valread = read(sock, buffer, buffer_size);
    printf("%s\n", buffer);
    
    // Close socket
    close(sock);
    sleep(0.1);
}

void *send_pkg(void *arg){
    struct thread_args *my_args = (struct thread_args *)arg;
    char * json_str = my_args->json_str;
    int cycles = my_args->cycles;
    int port = my_args->port;
	char * ip = my_args->ip;

    for (int i = 0; i < cycles ; i++){
        sem_wait(&semaphore);
        new_connection(json_str, port, ip);
        //sleep(3);
        sem_post(&semaphore);     
    }
}


int main(int argc, char *argv[]) {

	// Inicializa las variables
	int port = -1;
	int threads = -1;
	int cycles = -1;

    char image_path[100] = "";
	char* image_name = "";
    char ip[50] = "";

	char* image_base64;
    sem_init(&semaphore, 0, 1);

	char* key = generate_key();
	printf("Generated key: %s\n", key);

	json_t *data_to_send = json_object();  // Crear un objeto JSON vacío que almacenará la información 

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

	// Valida que las imagenes sea válidas y la convierte a base 64
	if (is_valid_image_path(image_path) == 1){
		printf("Imagen válida\n");
		image_base64 = image_to_base64(image_path);
		if (image_base64 == NULL) {
			printf("Error al obtener la imagen en base64\n");
			return 1;
		}
	}

	// Obtiene el nombre de la imagen

	image_name = get_filename(image_path);

    


	// rellenar información necesaria para ser enviada en el json

	json_object_set_new(data_to_send, "nombre", json_string(image_name));  // Agregar una cadena con clave "nombre"
    json_object_set_new(data_to_send, "data", json_string(image_base64));  // Agregar un string base 64 con clave "data"
    json_object_set_new(data_to_send, "key", json_string(key));  // Agregar un string con clave "key"
	json_object_set_new(data_to_send, "total", json_integer(threads*cycles));   // Agregar un entero con clave "total"
    
    char *json_str = json_dumps(data_to_send, JSON_ENCODE_ANY);  // Convertir el objeto JSON en una cadena JSON

    
    //printf("json: %s\n", json_str);  // Imprimir la cadena JSON
    
    // Send message to server
    for (int  i = 0 ; i < threads ; i++){

        struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));
        args->json_str = json_str;
        args->cycles = cycles;
        args->port = port;
		args->ip = ip;

        pthread_t thread;
        pthread_create(&thread, NULL, send_pkg, args);
        pthread_join(thread, NULL);

        //send_pkg(args);
    }
    
    return 0;
	
}






// Función que verifica si un string es un path válido a una imagen
int is_valid_image_path(char *image_path) {
    char *ext = strrchr(image_path, '.');  // Obtener la extensión del archivo
    if (ext == NULL) {
        return 0;
    }
    ext++;  // Incrementar el puntero para omitir el punto

    // Convertir la extensión a minúsculas para compararla con los formatos válidos
    for (char *p = ext; *p; p++) {
        *p = tolower(*p);
    }

    // Verificar si la extensión es válida y si el archivo existe
    if (strcmp(ext, "png") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
		FILE* fp = fopen(image_path, "rb");
    	if (!fp) 
		{
	        printf("No se pudo abrir la imagen: %s\n", image_path);
	        return 0;
		}
        return 1;
    }

    return 1;
}

char* image_to_base64(char* image_path) {
    // Leer la imagen y obtener su tamaño
    FILE* fp = fopen(image_path, "rb");
    if (!fp) {
        printf("No se pudo abrir la imagen: %s\n", image_path);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size_t image_size = ftell(fp);
    rewind(fp);

    // Leer la imagen en un buffer
    unsigned char* image_buffer = malloc(sizeof(unsigned char) * image_size);
    if (!image_buffer) {
        printf("No se pudo reservar memoria para el buffer de la imagen\n");
        fclose(fp);
        return NULL;
    }
    size_t bytes_read = fread(image_buffer, 1, image_size, fp);
    if (bytes_read != image_size) {
        printf("No se pudo leer la imagen correctamente\n");
        fclose(fp);
        free(image_buffer);
        return NULL;
    }
    fclose(fp);

    // Convertir la imagen a base64
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    bio = BIO_new(BIO_s_mem());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_write(bio, image_buffer, image_size);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);

    char* base64_string = malloc(sizeof(char) * (bufferPtr->length + 1));
    if (!base64_string) {
        printf("No se pudo reservar memoria para el string de base64\n");
        BIO_free_all(bio);
        free(image_buffer);
        return NULL;
    }
    memcpy(base64_string, bufferPtr->data, bufferPtr->length);
    base64_string[bufferPtr->length] = '\0';

    BIO_free_all(bio);
    free(image_buffer);


    return base64_string;
}

char* generate_key(void) {
    char* key = (char*)malloc((KEY_LENGTH + 1) * sizeof(char));
    memset(key, 0, (KEY_LENGTH + 1) * sizeof(char));
    srand((unsigned int)time(NULL)); // set seed based on current time

    for (int i = 0; i < KEY_LENGTH; i++) {
        key[i] = (char)(rand() % 26 + 'A'); // generate random uppercase letter
    }

    return key;
}

char* get_filename(char* path) {
    // Find last occurrence of "/" or "\" in the path
    char* separator = strrchr(path, '/');
    if (separator == NULL) {
        separator = strrchr(path, '\\');
    }

    // If separator found, return pointer to next character
    if (separator != NULL) {
        return separator + 1;
    }

    // If no separator found, return path
    return path;
}

