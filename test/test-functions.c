#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// #define MAXFILENUM 20
// #define MAXDIRNUM 10
#define MAX_PATH_SIZE 256
#define MAX_SS_NUM 10

// typedef struct storage_server
// {
//     int id;
//     char *files[MAXFILENUM];
//     char *dir[MAXDIRNUM];
// } ss;

struct CombinedFilesInfo
{
    int storageServerID;
    int numberOfFiles;
    int numberOfDirectories;
    struct FileInfo *files;            // Pointer to all the file names
    struct DirectoryInfo *directories; // Pointer to all the directory names
};

//* File Name with relative path to the directory where storage server was started from
struct FileInfo
{
    char name[MAX_PATH_SIZE];
};

//* Directory Name with relative path to the directory where storage server was started from
struct DirectoryInfo
{
    char name[MAX_PATH_SIZE];
};

struct CombinedFilesInfo allCombinedFilesInfo[MAX_SS_NUM];

int fileSearch(char *searchfilename)
{
    int i, j, k;
    int ssid = -1;
    for (i = 0; i < MAX_SS_NUM; i++)
    {
        // inside each storage server
        for (j = 0; j < allCombinedFilesInfo[i].numberOfFiles; j++)
        {
            // checking each file path

            // retrieving file name from path
            char *filename = strrchr(allCombinedFilesInfo[i].files[j].name, '/');
            if (filename == NULL)
                strcpy(filename, allCombinedFilesInfo[i].files[j].name);

            // comparing file name with searchfilename
            if (strcmp(filename, searchfilename) == 0)
            {
                ssid = i;
                break;
            }
        }
        if (ssid >= 0)
            break;
    }

    if (ssid == -1)
        printf("File not found in any storage server\n");
    else
        printf("File found on storage server %d\n", ssid);

    return ssid;
}

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
    // nwc testing
    // printf("Enter IP: ");
    // char ip[16];
    // scanf("%s", ip);
    // printf("Enter port: ");
    // int port;
    // scanf("%d", &port);
    // nwc(port, ip);

    // search for file
    printf("Enter file name: ");
    char *searchfilename = (char *)malloc(100 * sizeof(char));
    scanf("%s", searchfilename);
    printf("Searching for %s\n", searchfilename);
    int ssid = fileSearch(searchfilename);
}