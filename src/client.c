#include "../include/common.h"

struct ClientInfo client;

int initiateClient()
{
    int socket_client;
    struct sockaddr_in addr;
    socklen_t size_address;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_client < 0)
    {
        perror("[-]Socket error");
        close(socket_client);
        exit(1);
    }

    printf("[+]Client %s socket created.\n", client.clientName);

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LEADER_SERVER_PORT_CLIENT);
    addr.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);

    size_address = sizeof(addr);

    if (connect(socket_client, (struct sockaddr *)&addr, size_address) == 0)
    {
        printf("[+]Connected to Naming-Server.\n");
    }

    while (1)
    {
        int bytes_sent, bytes_received;
        struct ClientInfo client_info;
        bytes_sent = send(socket_client, &client_info, sizeof(client_info), 0);

        if (bytes_sent == -1)
        {
            perror("Error sending request to Naming-Server");
            close(socket_client);
            printf("[-]Connection to Naming-Server lost.\n");
            break;
        }

        bytes_received = recv(socket_client, &client_info, sizeof(client_info), 0);
        if (bytes_received == -1)
        {
            perror("Error receiving response from Naming-Server");
            close(socket_client);
            printf("[-]Connection to Naming-Server lost.\n");
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Naming-Server.\n");
            close(socket_client);
            printf("[-]Connection to Naming-Server lost.\n");
            break;
        }
    }

    return 0;
}

int main()
{
    printf("Enter client name: ");
    scanf("%s", client.clientName);

    initiateClient();
}