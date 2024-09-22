// client.c
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include "mongoose.h"
#include "client.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_MESSAGE_LENGTH 256
const wchar_t CLASS_NAME[] = L"NotificationWindow";

char ip_address[16];
int port;

FileInfo files[MAX_FILES];
int file_count = 0;
FileInfo previous_files[MAX_FILES];
int previous_file_count = 0;
wchar_t download_directory[MAX_PATH];

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
                        descargar_archivo(files[i].filename);
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SetTextColor(hdc, RGB(208, 229, 20));
        SetBkMode(hdc, TRANSPARENT);
        HFONT hFont = CreateFont(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFont);

        const TCHAR* text = (const TCHAR*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
    } break;

    case WM_DESTROY: {
        PostQuitMessage(0);
    } break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void notificar(const char* message) {
    wchar_t w_message[MAX_MESSAGE_LENGTH];
    MultiByteToWideChar(CP_UTF8, 0, message, -1, w_message, MAX_MESSAGE_LENGTH);

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        CLASS_NAME,
        L"Notificación",
        WS_POPUP,
        GetSystemMetrics(SM_CXSCREEN) - 200,
        GetSystemMetrics(SM_CYSCREEN) - 80,
        180, 50,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL) {
        return;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)w_message);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    Sleep(1300);

    DestroyWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void descargar_archivo(const char* filename) {
    SOCKET sock;
    struct sockaddr_in server;
    char source_url[256];
    char buffer[4096];
    int bytes_received;
    FILE* fp;

    snprintf(source_url, sizeof(source_url), "http://%s:%d/%s", ip_address, port, filename);

    char* ip = strtok(source_url + 7, ":");
    char* port_str = strtok(NULL, "/");
    int port_number = atoi(port_str);
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error al crear el socket: %d\n", WSAGetLastError());
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);
    inet_pton(AF_INET, ip, &server.sin_addr);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Error al conectar al servidor: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    char request[512];
    snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", filename, ip);
    send(sock, request, strlen(request), 0);

    char destination_path[MAX_PATH];
    snprintf(destination_path, sizeof(destination_path), "%s\\%s", download_directory, filename);
    fp = fopen(destination_path, "wb");
    if (fp == NULL) {
        printf("Error al crear el archivo: %s\n", destination_path);
        closesocket(sock);
        WSACleanup();
        return;
    }

    int header_received = 0;
    int header_length = 0;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        if (!header_received) {
            for (int i = 0; i < bytes_received; i++) {
                if (buffer[i] == '\r' && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' && buffer[i + 3] == '\n') {
                    header_length = i + 4;
                    header_received = 1;
                    break;
                }
            }
        }

        if (header_received) {
            fwrite(buffer + header_length, 1, bytes_received - header_length, fp);
            header_length = 0;
        }
    }

    fclose(fp);
    closesocket(sock);
    WSACleanup();
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

    printf("Ingrese la dirección IP del servidor: ");
    if (scanf("%15s", ip_address) != 1) {
        fprintf(stderr, "Error al leer la dirección IP.\n");
        return;
    }

    printf("Ingrese el puerto del servidor: ");
    if (scanf("%d", &port) != 1) {
        fprintf(stderr, "Error al leer el puerto.\n");
        return;
    }

    select_directory(download_directory);

    char server_url[32];
    snprintf(server_url, sizeof(server_url), "http://%s:%d", ip_address, port);

    mg_mgr_init(&mgr);
    system("cls");
    printf("Cliente iniciado. Escuchando... (CTRL + SHIFT + ALT + W para minimizar)\n");

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
