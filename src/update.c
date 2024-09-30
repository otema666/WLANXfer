#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>
#include "update.h"
#include <tchar.h>
#include <psapi.h>

#pragma comment(lib, "winhttp.lib")

#define APP_VERSION "1.1.1"
#define SERVER_NAME L"otema666.ddns.net"
#define SERVER_PATH L"WLANXfer/version.txt"
#define SERVER_PORT 443
#pragma warning(disable : 4996)

void clean_version(char* version) {
    char* end = version + strlen(version) - 1;
    while (end > version && (*end == '\n' || *end == '\r' || *end == ' ')) {
        *end = '\0';
        end--;
    }
}

void check_for_updates() {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;
    char server_version[11] = { 0 };
    DWORD dwStatusCode = 0;
    DWORD dwStatusCodeSize = sizeof(dwStatusCode);

    hSession = WinHttpOpen(L"WLANXfer/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        hConnect = WinHttpConnect(hSession, SERVER_NAME, SERVER_PORT, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", SERVER_PATH, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    }

    if (hRequest) {
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults) {
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwStatusCodeSize, NULL);
    }

    if (dwStatusCode == 200) {
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            printf("Error al consultar los datos disponibles.\n");
        }
        else if (dwSize > 0) {
            pszOutBuffer = (LPSTR)malloc(dwSize + 1);
            if (!pszOutBuffer) {
                printf("Memoria insuficiente.\n");
                return;
            }

            ZeroMemory(pszOutBuffer, dwSize + 1);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                printf("Error al leer los datos.\n");
            }
            else {
                strncpy(server_version, pszOutBuffer, dwDownloaded);
                server_version[dwDownloaded] = '\0';
                clean_version(server_version);
            }

            free(pszOutBuffer);
        }
    }
    else {
        printf("Error en la solicitud HTTP. Código de estado: %d\n", dwStatusCode);
		exit(1);
    }

    if (strcmp(APP_VERSION, server_version) != 0) {
        printf("Nueva versión disponible: %s. Presione ENTER para actualizar.\n", server_version);
		getchar();
        updateApp();
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}


void download_updater(const char* url, const char* output_filename) {
    HINTERNET hSession = WinHttpOpen(L"WLANXfer Updater/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET hConnect = NULL, hRequest = NULL;
    DWORD dwSize = 0, dwDownloaded = 0;
    BYTE buffer[1024];
    FILE* file = NULL;

    if (hSession) {
        hConnect = WinHttpConnect(hSession, L"otema666.ddns.net", 443, 0);
    }
    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/WLANXfer/updater.exe", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    }
    if (hRequest) {
        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            if (WinHttpReceiveResponse(hRequest, NULL)) {
                file = fopen(output_filename, "wb");
                if (file) {
                    do {
                        dwSize = 0;
                        if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                            if (WinHttpReadData(hRequest, buffer, min(dwSize, sizeof(buffer)), &dwDownloaded)) {
                                fwrite(buffer, 1, dwDownloaded, file);
                            }
                        }
                    } while (dwSize > 0);
                    fclose(file);
                }
            }
        }
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}

void updateApp() {
    char updater_path[MAX_PATH] = "updater.exe";
    char pid_str[10];
    DWORD pid = GetCurrentProcessId();

    download_updater("https://otema666.ddns.net/WLANXfer/updater.exe", updater_path);

    sprintf(pid_str, "%lu", pid);
    char command_line[256];
    sprintf(command_line, "\"%s\" %s", updater_path, pid_str);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        printf("Error al iniciar el updater.exe.\n");
    }
}