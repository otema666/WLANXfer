// client.c
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "mongoose.h"

#define SERVER_URL "http://192.168.1.60:8080"

void ev_handler_client(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        printf("Respuesta del servidor:\n%.*s\n", (int)hm->body.len, hm->body.buf);

        // analizar archivos
    }
}

void start_client() {
    struct mg_mgr mgr;
    struct mg_connection* c;

    mg_mgr_init(&mgr);

    printf("Cliente iniciado. Verificando el servidor cada 5 segundos...\n");

    while (1) {
        c = mg_http_connect(&mgr, SERVER_URL, ev_handler_client, NULL);
        if (c == NULL) {
            printf("Error al conectar con el servidor %s\n", SERVER_URL);
            break;
        }

        mg_printf(c, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", SERVER_URL);

        mg_mgr_poll(&mgr, 1000);

        Sleep(5000);
    }

    mg_mgr_free(&mgr);
}
