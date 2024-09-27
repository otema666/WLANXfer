#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

extern NOTIFYICONDATA nid; // Declaraci�n externa
extern volatile BOOL is_minimized; // Declaraci�n externa

void minimize_to_tray();
void restore_from_tray();
void delete_updater(const char* updater_path);

#endif // UTILS_H
