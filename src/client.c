// client.c
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include "mongoose.h"
#include "client.h"

FileInfo files[MAX_FILES];
int file_count = 0;
FileInfo previous_files[MAX_FILES];
int previous_file_count = 0;

void ev_handler_client(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;

        extract_filenames(hm->body.buf, files, &file_count);

        for (int i = 0; i < file_count; i++) {
            int found = 0;
            for (int j = 0; j < previous_file_count; j++) {
                if (strcmp(files[i].filename, previous_files[j].filename) == 0) {
                    found = 1;
                    if (files[i].timestamp != previous_files[j].timestamp) {
                        notificar(files[i].filename);
                    }
                    break;
                }
            }
            if (!found) {
                printf("\tNuevo archivo: %s \n", files[i].filename);
            }
        }

        memcpy(previous_files, files, sizeof(files));
        previous_file_count = file_count;

        c->is_closing = 1;
    }
}

void notificar(const char* filename) {
    NOTIFYICONDATAW nid;
    memset(&nid, 0, sizeof(NOTIFYICONDATAW));

    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = NULL;
    nid.uFlags = NIF_INFO | NIF_ICON;
    nid.uTimeout = 10000;
    nid.dwInfoFlags = NIIF_INFO;

    HICON hIcon = LoadImageW(NULL, L"res\\r_icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (hIcon == NULL) {
        DWORD error = GetLastError();
        printf("Error al cargar el icono: %lu\n", error);
        hIcon = LoadIconW(NULL, IDI_QUESTION);
    }
    nid.hIcon = hIcon;

    wchar_t info[256];
    wchar_t infoTitle[64];

    swprintf(info, sizeof(info) / sizeof(wchar_t), L"Archivo %S guardado con éxito", filename);

    MultiByteToWideChar(CP_UTF8, 0, "RStudio", -1, infoTitle, 64);
    
    wcsncpy_s(nid.szInfo, sizeof(nid.szInfo) / sizeof(wchar_t), info, _TRUNCATE);
    wcsncpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(wchar_t), infoTitle, _TRUNCATE);

    Shell_NotifyIconW(NIM_ADD, &nid);

    Shell_NotifyIconW(NIM_DELETE, &nid);

    if (hIcon) {
        DestroyIcon(hIcon);
    }
}

void extract_filenames(const char* html, FileInfo files[], int* file_count) {
    const char* href = "href=\"";
    const char* pos = html;
    *file_count = 0;

    while ((pos = strstr(pos, href)) != NULL) {
        pos += strlen(href);
        const char* end = strchr(pos, '"');
        if (end) {
            size_t len = end - pos;
            if (len < MAX_FILENAME_LEN) {
                if (pos[0] == '#' || (pos[0] == '.' && pos[1] == '.')) {
                    pos = end;
                    continue;
                }

                strncpy(files[*file_count].filename, pos, len);
                files[*file_count].filename[len] = '\0';

                const char* timestamp_start = strstr(end, "name=");
                if (timestamp_start) {
                    timestamp_start += strlen("name=");
                    files[*file_count].timestamp = strtoull(timestamp_start, NULL, 10);
                }
                else {
                    files[*file_count].timestamp = 0;
                }

                (*file_count)++;
                if (*file_count >= MAX_FILES) {
                    break;
                }
            }
        }
    }
}

void start_client() {
    struct mg_mgr mgr;
    struct mg_connection* c;
    char ip[16];
    int port;

    printf("Ingrese la dirección IP del servidor: ");
    if (scanf("%15s", ip) != 1) {
        fprintf(stderr, "Error al leer la dirección IP.\n");
        return 1;
    }

    printf("Ingrese el puerto del servidor: ");
    if (scanf("%d", &port) != 1) {
        fprintf(stderr, "Error al leer el puerto.\n");
        return 1;
    }

    char server_url[32];
    snprintf(server_url, sizeof(server_url), "http://%s:%d", ip, port);

    mg_mgr_init(&mgr);
    system("cls");
    printf("Cliente iniciado. Escuchando...\n");
    printf("Listado de archivos:\n");

    while (1) {
        c = mg_http_connect(&mgr, server_url, ev_handler_client, NULL);
        if (c == NULL) {
            printf("Error al conectar con el servidor %s\n", server_url);
        }

        mg_printf(c, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", server_url);

        mg_mgr_poll(&mgr, 1000);

        Sleep(500);
    }

    mg_mgr_free(&mgr);
}

