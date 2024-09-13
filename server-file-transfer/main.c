#include "mongoose.h"
#include <stdio.h>
#include <string.h>

void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;

        struct mg_http_serve_opts opts = {
          .root_dir = "./",
        };

        mg_http_serve_dir(c, hm, &opts);
    }
}

void parse_args(int argc, char* argv[], char* root_dir, char* port) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strcpy(root_dir, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            strcpy(port, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Uso: %s [-d root_dir] [-p port]\n", argv[0]);
            printf("Opciones:\n");
            printf("  -d root_dir  Especifica el directorio raíz.\n");
            printf("  -p port      Especifica el puerto.\n");
            printf("  -h           Muestra esta ayuda.\n");
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    struct mg_mgr mgr;
    struct mg_connection* c;
    char root_dir[100] = "./";
    char port[10] = "8000";

    parse_args(argc, argv, root_dir, port);

    mg_mgr_init(&mgr);

    char listen_addr[20];
    snprintf(listen_addr, sizeof(listen_addr), "http://0.0.0.0:%s", port);

    c = mg_http_listen(&mgr, listen_addr, ev_handler, NULL);
    if (c == NULL) {
        printf("Error al iniciar el servidor en el puerto %s\n", port);
        return 1;
    }

    printf("Servidor HTTP iniciado en el puerto %s, sirviendo el directorio %s\n", port, root_dir);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
