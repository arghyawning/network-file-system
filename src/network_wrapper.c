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

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0)
    {
        perror("[-]Error in binding");
        exit(1);
    }
    else
    {
        printf("[+]Bind successful.\n");
    }
}

void Listen(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) < 0)
    {
        perror("[-]Error in listening");
        exit(1);
    }
    else
    {
        printf("[+]Listening....\n");
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int new_socket;
    if ((new_socket = accept(sockfd, addr, addrlen)) < 0)
    {
        perror("[-]Error in accepting");
        exit(1);
    }
    else
    {
        printf("[+]Client connected.\n");
    }
    return new_socket;
}