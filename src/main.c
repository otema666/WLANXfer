// main.c
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "server.h"
#include "client.h"
#include "update.h"
#include "utils.h"

volatile BOOL keep_running = TRUE;


DWORD WINAPI monitor_keys(LPVOID lpParam) {
    while (keep_running) {
        if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
            if (!is_minimized) {
                minimize_to_tray();
            }
            else {
                restore_from_tray();
            }
        }

        while ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
            Sleep(100);
        }
        Sleep(40);
    }
    return 0;
}

BOOL WINAPI console_handler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        keep_running = FALSE;
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        const char* updater_path = argv[1];
        delete_updater(updater_path);
    }

    SetConsoleTitle(L"WLANXfer");
    setlocale(LC_ALL, "");

	check_for_updates();
    system("pause");
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        fprintf(stderr, "Error al instalar el manejador de señales.\n");
        return EXIT_FAILURE;
    }

    char option[10];
    char port[10];

    HANDLE hThread = CreateThread(NULL, 0, monitor_keys, NULL, 0, NULL);
    if (hThread == NULL) {
        fprintf(stderr, "Error al crear el hilo para monitor de teclas.\n");
        return EXIT_FAILURE;
    }

    while (TRUE) {
        system("cls");
        setColor(11);
        printf("\t\t\t\t============================================\n");
        printf("\t\t\t\t                WLANXfer MENU               \n");
        printf("\t\t\t\t============================================\n");

        setColor(14);
        printf("1: Iniciar como Host\n");
        printf("2: Iniciar como Cliente\n");
        printf("3: Salir\n");
        setColor(7);

        printf("\nSeleccione una opción: ");
        fgets(option, sizeof(option), stdin);
        printf("\n");

        if (option[0] == '1') {
            input_port(port);
            select_directory(global_root_dir);
            start_server(port);
        }
        else if (option[0] == '2') {
            setColor(14);
            printf("Iniciando modo cliente...\n");
            setColor(7);
            system("cls");
            start_client();
        }
        else if (option[0] == '3') {
            setColor(12);
            printf("Saliendo...\n");
            setColor(7);
            keep_running = FALSE;
            system("pause");
            break;
        }
        else {
            setColor(12);
            printf("Opción no válida.\n");
            setColor(7);
            system("pause");
        }
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    setColor(7);
    return 0;
}
