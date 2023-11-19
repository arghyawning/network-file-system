#include "../include/common.h"

void close_socket(int socket)
{
    if (close(socket) < 0)
    {
        perror("[-]Error closing socket");
    }
    else
    {
        printf("[+]Socket closed.\n");
    }
}