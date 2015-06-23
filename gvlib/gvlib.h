#ifndef GV_LIB_H
#define GV_LIB_H

#include <unistd.h>
#include <socket.h>
#include <string.h>

typedef struct {
    int so_ctrl;
    int so_data;
    struct sockaddr_un so_addr;
} gv_socket_t;


int getdatasocket(socket_t *sd);
int getgvsocketunix(char *su_path);


#endif /* gv_lib.h */

int getdatasocket(socket_t *sd)
{
    return sd->so_data;
}


int getgvsocketunix(char *su_path)
{
    strcpy(su_path, "/var/gaver.sock");
}

