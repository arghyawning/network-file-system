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
#include <libgen.h>

#include "colours.h"
#include "ss.h"
#include "naming_server.h"
#include "shell.h"

#define LEADER_SERVER_IP "127.0.0.1"
#define STORAGE_SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"

#define LEADER_SERVER_PORT_CLIENT 8000
#define LEADER_SERVER_PORT 8080
#define SERVER_BACKLOG 10
#define MAX_PORTS_PER_SS 4
#define MAX_PORTS_PER_CLIENT 3
#define MAX_CONNECTION_TRIES 4
#define BASE_PORT_SS 5000
#define BASE_PORT_CLIENT 6000

#define READ_SIZE 1024

#define SERIALIZED_BUFFER_SIZE 8092
#define MAX_PATH_SIZE 256
#define MAX_BUFFER_SIZE 1024
#define MAX_USERNAME_SIZE 32
#define MAX_COMMAND_NAME 16
#define MAX_INPUT_LENGTH (MAX_COMMAND_NAME + MAX_PATH_SIZE * 2 + 2)
#define MAX_COMMAND_ARGUMENTS 2
#define MAX_CHUNK_SIZE 16

#define MAX_FILES 32

#define MAX_SS 16
#define MAX_CLIENTS 16
#define MAX_REDUNDANT_SS 2

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
    char ss_boot_path[MAX_PATH_SIZE];
    int redundant_ss[MAX_REDUNDANT_SS];
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
    char clientName[MAX_USERNAME_SIZE];
    int sessionID;
    char ipAddress[16];
    int clientPort;
    int isConnected;
};

struct ClientRequest
{
    uint64_t transactionId;
    char command[MAX_COMMAND_NAME];
    char arguments[MAX_COMMAND_ARGUMENTS][MAX_BUFFER_SIZE];
    int clientID;
};

struct Client_to_NM_response
{
    uint64_t transactionId;
    int operation_performer; //* 1 for NM and 2 for SS
    int ss_port;
    char ss_ip[16];
    struct FileInfo file;
};

struct Client_to_SS_Request
{
    char buffer[64];
    char command[MAX_COMMAND_NAME];
    struct FileInfo file;
    int write_type;
};

// struct Client_to_SS_Response{

// };

struct Packet
{
    int isStart; // 1 if start of file, 0 otherwise
    int isEnd;   // 1 if end of file, 0 otherwise
    int packetNumber;
    char data[READ_SIZE];
};

struct PacketWrite
{
    int isStart;      // 1 if start of file, 0 otherwise
    int isEnd;        // 1 if end of file, 0 otherwise
    int packetNumber; // Packet number for tracking
    int write_type;   // 1 for overwrite, 2 for append
    char data[MAX_CHUNK_SIZE];
};

void scan_dir(struct FileInfo **files, struct DirectoryInfo **directories, struct NumberOfFiles *fileInfo);

#endif