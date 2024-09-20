// main.c
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "server.h"
#include "client.h"

NOTIFYICONDATA nid;
volatile BOOL keep_running = TRUE;
volatile BOOL is_minimized = FALSE;

void minimize_to_tray() {
    HWND hWnd = GetConsoleWindow();

    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 100;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
    wcscpy_s(nid.szTip, _countof(nid.szTip), L"WLANXfer");
    nid.uCallbackMessage = WM_USER + 1;

    Shell_NotifyIcon(NIM_ADD, &nid);

    ShowWindow(hWnd, SW_HIDE);
    is_minimized = TRUE;
}

void restore_from_tray() {
    HWND hWnd = GetConsoleWindow();

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    Shell_NotifyIcon(NIM_DELETE, &nid);
    is_minimized = FALSE;
}

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

int main() {
    SetConsoleTitle(L"WLANXfer");

    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        fprintf(stderr, "Error al instalar el manejador de señales.\n");
        return EXIT_FAILURE;
    }

    setlocale(LC_ALL, "");
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
