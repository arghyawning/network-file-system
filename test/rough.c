#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void nwc(int port, char *ip)
{
    int sock;
    struct sockaddr_in serverinfo;
    socklen_t addr_size;

    sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock < 0)
    {
        perror("socket");
        return;
    }

    memset(&serverinfo, '\0', sizeof(serverinfo)); // Zero out structure
    serverinfo.sin_family = AF_INET;               // Internet Protocol v4 addresses
    serverinfo.sin_port = htons(port);             // Port number
    serverinfo.sin_addr.s_addr = inet_addr(ip);    // IP address

    addr_size = sizeof(serverinfo);
    if (connect(sock, (struct sockaddr *)&serverinfo, addr_size) < 0)
    {
        perror("connect");
        close(sock);
        return;
    }

    printf("Connected to %s:%d\n", ip, port);
    close(sock);
}

int main()
{
    printf("Enter IP: ");
    char ip[16];
    scanf("%s", ip);
    printf("Enter port: ");
    int port;
    scanf("%d", &port);
    nwc(port, ip);
}