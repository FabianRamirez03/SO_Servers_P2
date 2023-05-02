#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <png.h>
#include <jpeglib.h>
#include <openssl/evp.h>
#include <openssl/bio.h>

int base64_to_image(const char *base64_string, const char *key, const char* path)
{
    // const char *path_to_save = "./Servers/heavy_db/temp.png";
    char result_name[256];
    snprintf(result_name, sizeof(result_name), "%s/tmp/tmp_%s.png",path,  key);
    const char *path_to_save = result_name;

    BIO *bio, *b64;
    FILE *fp;
    int image_size = strlen(base64_string);
    char *buffer = (char *)malloc(image_size);
    memset(buffer, 0, image_size);

    bio = BIO_new_mem_buf((void *)base64_string, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_read(bio, buffer, image_size);

    if (access(path_to_save, F_OK) == 0)
    {
        BIO_free_all(bio);
        free(buffer);
        return -1;
    }

    fp = fopen(path_to_save, "wb");
    if (!fp)
    {
        printf("No se pudo abrir el archivo de imagen para escritura\n");
        BIO_free_all(bio);
        free(buffer);
        return -1;
    }

    fwrite(buffer, 1, image_size, fp);
    fclose(fp);

    BIO_free_all(bio);
    free(buffer);

    return 0;
}