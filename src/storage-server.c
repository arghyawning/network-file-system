#include "../include/common.h"
struct StorageServer
{
    char ipAddress[15]; // IPv4 address
    int nameServerPort;
    int clientPort;
    char accessiblePaths[100][50]; // max 100 paths, each of max length 50
};
void initializeSS(struct StorageServer *ss, const char *ip, int nmPort, int clPort, const char *paths[]);
void sendDetailsToNamingServer(struct StorageServer *ss);

void initializeSS(struct StorageServer *ss, const char *ip, int nmPort, int clPort, const char *paths[])
{
    int i;
    strcpy(ss->ipAddress, ip);
    ss->nameServerPort = nmPort;
    ss->clientPort = clPort;

    for (i = 0; i < 100 && paths[i] != NULL; i++)
    {
        strcpy(ss->accessiblePaths[i], paths[i]);
    }

    sendDetailsToNamingServer(ss);
}

void sendDetailsToNamingServer(struct StorageServer *ss)
{
    int i;
    printf("Storage Server Details Sent to Naming Server:\n");
    printf("IP: %s\n", ss->ipAddress);
    printf("NM Port: %d\n", ss->nameServerPort);
    printf("Client Port: %d\n", ss->clientPort);
    printf("Paths:\n");
    for (int i = 0; i < 100 && strlen(ss->accessiblePaths[i]) > 0; ++i)
    {
        printf("- %s\n", ss->accessiblePaths[i]);
    }
}

int main()
{
    struct StorageServer ss1;

    initializeSS(&ss1, "192.168.1.1", 5001, 8001, (const char *[]){"/path1", "/path2", NULL});

    return 0;
}
