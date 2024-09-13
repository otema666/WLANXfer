#ifndef SERVER_H
#define SERVER_H
#include <windows.h>

extern char global_root_dir[MAX_PATH];
extern volatile BOOL keep_running;

void setColor(int color);
void input_port(char* port);
void launch_url_qr(const char* url);
void select_directory(char* root_dir);
void ev_handler(struct mg_connection* c, int ev, void* ev_data);
void get_local_ip(char* ip);
void start_server(char* port);

#endif
