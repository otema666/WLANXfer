#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#include <wininet.h>
#include <winhttp.h>
#include <psapi.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")

#define MAX_BUFFER_SIZE 8192
#define MAX_RETRIES 3
#define RETRY_DELAY 3500
#pragma warning(disable : 4996)

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void kill_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        setColor(12);
        printf("Error al abrir el proceso con PID: %lu\n", pid);
		setColor(7);
        return;
    }
    if (!TerminateProcess(hProcess, 0)) {
		setColor(12);
        printf("Error al terminar el proceso con PID: %lu\n", pid);
		setColor(7);
    }
    else {
		setColor(10);
        printf("[+] Proceso con PID %lu terminado.\n", pid);
		setColor(7);
    }
    CloseHandle(hProcess);
}



BOOL DownloadFile(const char* url, const char* localPath) {
    BOOL result = FALSE;
    HINTERNET hSession = WinHttpOpen(L"Downloader", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/otema666/WLANXfer/releases/latest/download/WLANxter-x64.exe", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

        if (hRequest) {
            if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                if (WinHttpReceiveResponse(hRequest, NULL)) {
                    DWORD bytesRead = 0;
                    DWORD totalBytesRead = 0;
                    char buffer[MAX_BUFFER_SIZE];

                    FILE* file = fopen(localPath, "wb");
                    if (file) {
                        do {
                            if (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead)) {
                                fwrite(buffer, 1, bytesRead, file);
                                totalBytesRead += bytesRead;
                                printf("Descargando... %lu bytes leídos\r", totalBytesRead);
                            }
                        } while (bytesRead > 0);
                        fclose(file);
                        setColor(10);
                        printf("\n[+] Descarga completada.\n");
                        setColor(7);
                        result = TRUE;
                    }
                    else {
                        printf("Error al abrir el archivo para escribir.");
                    }
                }
                else {
                    printf("Error al recibir la respuesta. Código de error: %lu", GetLastError());
                }
            }
            else {
                printf("Error al enviar la solicitud. Código de error: %lu", GetLastError());
            }
            WinHttpCloseHandle(hRequest);
        }
        else {
            printf("Error al abrir la solicitud. Código de error: %lu", GetLastError());
        }
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
    }
    else {
        printf("Error al abrir la sesión de WinHttp. Código de error: %lu", GetLastError());
    }
    return result;
}

void download_new_executable() {
    char url[256];
    sprintf(url, "https://github.com/otema666/WLANXfer/releases/latest/download/WLANxter-x64.exe");

    const char* localPath = "WLANxter.exe";
    int attempts = 0;

    while (attempts < MAX_RETRIES) {
        if (DownloadFile(url, localPath)) {
            printf("El archivo se descargó correctamente.\n");
            break;
        }
        else {
            attempts++;
            printf(" Intentando de nuevo en %d segundos...\n", RETRY_DELAY / 1000);
            Sleep(RETRY_DELAY);
        }
    }

    if (attempts == MAX_RETRIES) {
        printf("Error: No se pudo descargar el ejecutable después de %d intentos.\n", MAX_RETRIES);
        exit(1);
    }
}


void execute_new_executable(const char* filename, const char* updater_path) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char command_line[MAX_PATH + 50];
    sprintf(command_line, "\"%s\" \"%s\"", filename, updater_path);

    if (CreateProcessA(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        printf("Nuevo ejecutable iniciado en una nueva consola: %s\n", filename);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        printf("Error al iniciar el nuevo ejecutable.\n");
    }
}

int main(int argc, char* argv[]) {
    SetConsoleTitle(L"WLANXfer updater");
    setlocale(LC_ALL, "");
	system("cls");
    if (argc != 2) {
        printf("Uso: updater.exe {pid}\n");
        return 1;
    }
    
    int pid = atoi(argv[1]);

    if (pid == 0 && argv[1][0] != '0') {
        printf("Error: PID no válido.\n");
        return 1;
    }
    setColor(11);
    printf("\t\t\t\t============================================\n");
    printf("\t\t\t\t           Iniciando actualización          \n");
    printf("\t\t\t\t============================================\n\n");

    setColor(7);
    char updater_path[MAX_PATH];
    GetModuleFileName(NULL, updater_path, MAX_PATH);

    snprintf(updater_path, sizeof(updater_path), "%s", updater_path);

    kill_process(pid);
	printf("\n");
    download_new_executable();
	printf("\n");
	system("pause");
    execute_new_executable("WLANxter-x64.exe", (const char*)updater_path);
    return 0;
}