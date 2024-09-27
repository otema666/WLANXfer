#include "utils.h"

NOTIFYICONDATA nid;
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

void delete_updater(const char* updater_path) {
    char command[MAX_PATH];
    sprintf(command, "cmd.exe /C del \"%s\"", updater_path);
    WinExec(command, SW_HIDE);
}