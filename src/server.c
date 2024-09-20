// server.c
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <wininet.h>
#include "mongoose.h"
#include "server.h"

#pragma comment(lib, "wininet.lib")

char global_root_dir[MAX_PATH];

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void input_port(char* port) {
    printf("Puerto (1024-49151): ");
    setColor(10);
    fgets(port, 10, stdin);
    setColor(7);
    port[strcspn(port, "\n")] = 0;
    if (atoi(port) == 0) {
        strcpy_s(port, 10, "80");
        printf("Seleccionado puerto 80 por defecto.\n");
    }
}

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void launch_url_qr(const char* url) {
    HINTERNET hInternet, hConnect;
    FILE* fp;
    const char* filename = "QR.png";
    char url_qr[256];
    _snprintf_s(url_qr, sizeof(url_qr), _TRUNCATE, "https://api.qrserver.com/v1/create-qr-code/?data=%s", url);
    DWORD bytesRead;
    BYTE buffer[4096];

    hInternet = InternetOpen(L"QR Code Downloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        fprintf(stderr, "InternetOpen failed: %d\n", GetLastError());
        return;
    }
    hConnect = InternetOpenUrlA(hInternet, url_qr, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        fprintf(stderr, "InternetOpenUrlA failed: %d\n", GetLastError());
        InternetCloseHandle(hInternet);
        return;
    }
    errno_t err = fopen_s(&fp, filename, "wb");
    if (err != 0) {
        perror("Failed to open file");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, fp);
    }

    fclose(fp);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    ShellExecuteA(NULL, "open", filename, NULL, NULL, SW_SHOWNORMAL);

    Sleep(1200);
    DeleteFileA(filename);
}

void select_directory(char* root_dir) {
    printf("Directorio: ");
    IFileDialog* pfd = NULL;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileDialog, (void**)&pfd);
        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            pfd->lpVtbl->GetOptions(pfd, &dwOptions);
            pfd->lpVtbl->SetOptions(pfd, dwOptions | FOS_PICKFOLDERS);

            hr = pfd->lpVtbl->Show(pfd, NULL);
            if (SUCCEEDED(hr)) {
                IShellItem* psiResult;
                hr = pfd->lpVtbl->GetResult(pfd, &psiResult);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFolderPath = NULL;
                    hr = psiResult->lpVtbl->GetDisplayName(psiResult, SIGDN_FILESYSPATH, &pszFolderPath);

                    if (SUCCEEDED(hr)) {
                        size_t convertedChars = 0;
                        wcstombs_s(&convertedChars, root_dir, MAX_PATH, pszFolderPath, _TRUNCATE);

                        setColor(10); // 10 = verde
                        printf("%s\n", root_dir);

                        setColor(7); // 7 = color predeterminado (blanco)

                        CoTaskMemFree(pszFolderPath);
                    }
                    psiResult->lpVtbl->Release(psiResult);
                }
            }
            pfd->lpVtbl->Release(pfd);
        }
        CoUninitialize();
    }
}

void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;

        struct mg_http_serve_opts opts = {
            .root_dir = global_root_dir,
        };

        mg_http_serve_dir(c, hm, &opts);
    }
}

void get_local_ip(char* ip, size_t ip_size) {
    WSADATA wsaData;
    struct addrinfo hints, * res;
    char hostname[256];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        strncpy_s(ip, ip_size, "Error", _TRUNCATE);
        return;
    }

    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        strncpy_s(ip, ip_size, "Error", _TRUNCATE);
        WSACleanup();
        return;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        strncpy_s(ip, ip_size, "Error", _TRUNCATE);
        WSACleanup();
        return;
    }

    struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ip, ip_size);

    freeaddrinfo(res);
    WSACleanup();
}


void start_server(char* port) {
    struct mg_mgr mgr;
    struct mg_connection* c;

    mg_mgr_init(&mgr);

    char listen_addr[20];
    _snprintf_s(listen_addr, sizeof(listen_addr), _TRUNCATE, "http://0.0.0.0:%s", port);

    c = mg_http_listen(&mgr, listen_addr, ev_handler, NULL);
    if (c == NULL) {
        printf("Error al iniciar el servidor en el puerto %s\n", port);
        mg_mgr_free(&mgr);
        return;
    }
	printf("Generando código qr...\n");
    char ip[20];
    get_local_ip(ip, sizeof(ip));
    char url[50];
    _snprintf_s(url, sizeof(url), _TRUNCATE, "http://%s:%s", ip, port);
    launch_url_qr(url);
    system("cls");

    printf("Servidor HTTP iniciado. (CTRL + SHIFT + ALT + W para minimizar)\n");
    setColor(1); printf("%s\n", url); setColor(7);
    
    //char command[100];
    //_snprintf_s(command, sizeof(command), _TRUNCATE, "start %s", url);
    //system(command);
    printf("\nPresione Ctrl+C para detener el servidor.\n");

    while (keep_running) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    setColor(12);
    printf("\nServidor detenido.\n");
    setColor(7);
    system("pause");
}


