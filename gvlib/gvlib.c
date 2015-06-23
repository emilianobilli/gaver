#include "gvlib.h"

int getdatasocket(socket_t *sd)
{
    return sd->so_data;
}

int getgvsocketunix(char *su_path)
{
    strcpy(su_path, "/var/gaver.sock");
}

