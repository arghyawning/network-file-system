#ifndef _COMMON_H_
#define _COMMON_H_

//* All are part of POSIX
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "colours.h"
#include "ss.h"
#include "naming_server.h"
#include "shell.h"

#define LEADER_SERVER_IP "127.0.0.1"
#define STORAGE_SERVER_IP "127.0.0.1"

#define LEADER_SERVER_PORT_CLIENT 8000
#define LEADER_SERVER_PORT 8080
#define SERVER_BACKLOG 10
#define MAX_PORTS_PER_SS 4
#define MAX_PORTS_PER_CLIENT 3
#define MAX_CONNECTION_TRIES 4
#define BASE_PORT_SS 5000
#define BASE_PORT_CLIENT 6000

#define SERIALIZED_BUFFER_SIZE 4096
#define MAX_PATH_SIZE 256
#define MAX_BUFFER_SIZE 1024

#define MAX_FILES 32

#define MAX_SS 16
#define MAX_CLIENTS 16

#define HEARTBEAT_PERIOD 2
#define RETRY_INTERVAL 1

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

//* Number of files and directories per Storage Server (to return multiple values from scan_directory)
struct NumberOfFiles
{
    int numberOfFiles;
    int numberOfDirectories;
};

//* Storage Server Info
struct StorageServerInfo
{
    int storageServerID;
    char ipAddress[16];
    int storageServerPort;
    int clientPort;
    int heartbeatPort;
    int isConnected;
    int numberOfFiles;
    int numberOfDirectories;
};

//? can remove this
struct StorageServerHeartBeat
{
    char buffer[64];
    int ss_id;
    int heartbeatCount;
};

//* Combined Files Info per Storage Server
struct CombinedFilesInfo
{
    int storageServerID;
    int numberOfFiles;
    int numberOfDirectories;
    struct FileInfo *files;            //* Pointer to all the file names
    struct DirectoryInfo *directories; //* Pointer to all the directory names
};

struct ClientInfo
{
    int clientID;
    char clientName[64];
    char ipAddress[16];
    int clientPort;
    int isConnected;
};

struct ClientRequest
{
    char buffer[64];
    int clientID;
};

void scan_dir(struct FileInfo **files, struct DirectoryInfo **directories, struct NumberOfFiles *fileInfo);

#endif