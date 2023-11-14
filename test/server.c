#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

const int PORT = 8080;
// const char *IP = "127.0.0.1";
const char *IP = "192.168.2.204";

#define CHUNK_SIZE 256

struct datapack
{
    int index;
    char data[CHUNK_SIZE];
    int numchunks;
};

int main()
{
    int sock;
    struct sockaddr_in server_address, client_address;
    int n;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("[-]socket error");
        exit(1);
    }

    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(IP);

    n = bind(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (n < 0)
    {
        perror("[-]bind error");
        exit(1);
    }

    char **message;

    struct datapack rdp;
    socklen_t address_size = sizeof(client_address);
    printf("[+]Server running...\n");
    int count = 0, chunksnum = 1;
    while (count < chunksnum)
    {
        n = recvfrom(sock, &rdp, sizeof(rdp), 0, (struct sockaddr *)&client_address, &address_size);
        if (n < 0)
        {
            perror("[-]recvfrom error ");
            exit(1);
        }
        // printf("[+]Data received in chunk %d: %s\n", rdp.index, rdp.data);
        chunksnum = rdp.numchunks;
        if (count == 0)
            message = (char **)malloc(rdp.numchunks * sizeof(char *));
        count++;

        int index;
        index = rdp.index;

        message[index] = (char *)malloc((CHUNK_SIZE + 1) * sizeof(char));
        strcpy(message[index], rdp.data);

        // Here, the server should send back an acknowledgment to the client
        n = sendto(sock, &index, sizeof(int), 0, (struct sockaddr *)&client_address, address_size);
        if (n < 0)
        {
            perror("[-]sendto error");
            exit(1);
        }
    }

    FILE *op = fopen("storage/testsave.txt", "w");
    if (op == NULL)
    {
        perror("[-]fopen error");
        exit(1);
    }

    printf("Message received: \n");
    for (int i = 0; i < chunksnum; i++)
    {
        printf("%s", message[i]);
        fprintf(op, "%s", message[i]);
    }
    fclose(op);
    printf("\n");

    close(sock);

    return 0;
}