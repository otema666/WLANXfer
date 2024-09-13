// main.c
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "server.h"


volatile BOOL keep_running = TRUE;

BOOL WINAPI console_handler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        keep_running = FALSE;
        return TRUE;
    }
    return FALSE;
}

int main() {
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        fprintf(stderr, "Error al instalar el manejador de señales.\n");
        return EXIT_FAILURE;
    }

    setlocale(LC_ALL, "");
    char option[10];
    char port[10];

    while (TRUE) {
        setColor(11);
        printf("============================================\n");
        printf("                SERVER MENU                 \n");
        printf("============================================\n");

        setColor(14);
        printf("1: Iniciar como Host\n");
        printf("2: Modo Cliente (no implementado)\n");
        setColor(7);

        printf("Seleccione una opción: ");
        fgets(option, sizeof(option), stdin);
        printf("\n");

        if (option[0] == '1') {
            input_port(port);
            select_directory(global_root_dir);
            start_server(port);
        }
        else if (option[0] == '2') {
            setColor(12);
            printf("Modo cliente no implementado aún.\n");
            setColor(7);
        }
        else {
            setColor(12);
            printf("Opción no válida. Saliendo...\n");
            setColor(7);
            system("pause");
        }
    }

    setColor(7);
    return 0;
}
