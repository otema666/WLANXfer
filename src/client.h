// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include "server.h"

#define MAX_FILENAME_LEN 256
#define MAX_FILES 100

typedef struct {
    wchar_t filename[MAX_FILENAME_LEN];
    uint64_t timestamp;
} FileInfo;

void start_client();
void extract_filenames(const char* html, FileInfo files[], int* file_count);
void notificar(const char* message);
void descargar_archivo(const char* filename);

#endif
