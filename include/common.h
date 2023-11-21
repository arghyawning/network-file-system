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
#include "errors.h"

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
#define MAX_NEW_DIRECTORIES 8

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
    int copyPort;
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

struct CopyInfo
{
    int ss_source_id;
    int ss_destination_id;
    int ss_source_port;
    int ss_destination_port;
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
    int operation_status;
    struct CopyInfo copyInfo;
    int start_operation;
};

struct Client_to_NM_response
{
    uint64_t transactionId;
    int operation_performer; //* 1 for NM and 2 for SS
    int ss_port;
    char ss_ip[16];
    struct FileInfo file;
    int operation_status;
};

struct Client_to_SS_Request
{
    char buffer[64];
    char command[MAX_COMMAND_NAME];
    struct FileInfo file;
    int write_type;
};

struct NM_to_SS_Response
{
    int operation_status;
    struct DirectoryInfo dir[MAX_NEW_DIRECTORIES];
    struct FileInfo file[MAX_FILES];
    int new_dir_count;
    int new_file_count;
    int source_ss_confirmation;
    int destination_ss_confirmation;
    int start_operation;
};

struct Packet
{
    int isStart; // 1 if start of file, 0 otherwise
    int isEnd;   // 1 if end of file, 0 otherwise
    int packetNumber;
    char data[READ_SIZE + 1];
};

struct PacketWrite
{
    int isStart;      // 1 if start of file, 0 otherwise
    int isEnd;        // 1 if end of file, 0 otherwise
    int packetNumber; // Packet number for tracking
    int write_type;   // 1 for overwrite, 2 for append
    char data[MAX_CHUNK_SIZE];
    int datalength;
};

//* hashing.h
typedef struct file_found
{
    int ssid;                 // storage server id
    struct FileInfo filepath; // path of the file
} ff;

typedef struct directory_found
{
    int ssid;                     // storage server id
    struct DirectoryInfo dirpath; // path of the directory
} dd;

typedef struct per_key_bucket
{
    int key;             // length of filename
    ff files[MAX_FILES]; // files with same key
    int num_files;       // number of files with this particular key
} bucket;

typedef struct per_key_bucket_dir
{
    int key;                   // length of filename
    dd directories[MAX_FILES]; // files with same key
    int num_directories;       // number of files with this particular key
} bucket_dir;

void initialize_hash_table(bucket *fileshash, bucket_dir *dirhash);
void removeHashEntryFile(char *filename, bucket *fileshash);
void removeHashEntryDirectory(char *filename, bucket_dir *dirhash);
void store_in_hash_dir(struct CombinedFilesInfo *directories, bucket_dir *dirhash);
void store_in_hash_file(struct CombinedFilesInfo *files, bucket *fileshash);
void add_file_in_hash(char *filename, int ssid, bucket *fileshash);
void add_dir_in_hash(char *filename, int ssid, bucket_dir *dirhash);
dd dirSearchWithHash(char *searchfilename, bucket_dir *dirhash);
ff fileSearchWithHash(char *searchfilename, bucket *fileshash);
void print_hash_table_files(bucket *fileshash);
void print_hash_table_directories(bucket_dir *dirhash);

//* search_tree.h
struct DirTree
{
    // char name[MAX_PATH_SIZE];
    struct DirectoryInfo dirinfo; // directory name
    int numberOfFiles;            // number of files in this directory
    int numberOfDirectories;      // number of directories in this directory
    int ssid;                     // storage server id
    struct DirTree *directories;  // pointer to all the directories in this directory
    struct FileInfo *files;       // pointer to all the files in this directory
};

struct TreeClosestDirPacket
{
    int ssid;
    struct DirectoryInfo dirinfo;
};

void initialiseDirTree(struct DirTree *dirTree);
void fillDirTree(struct DirTree *dirTree, struct CombinedFilesInfo *combinedFilesInfoAll);
int searchDirTree(struct DirTree *dirTree, char *filename);
struct TreeClosestDirPacket closestDir(struct DirTree *dirTree, char *filename);
int removeFromTree(struct DirTree *dirTree, int type, char *delpath);

//* storage_server.h
void scan_dir(struct FileInfo **files, struct DirectoryInfo **directories, struct NumberOfFiles *fileInfo);

//* nm_utils.h
void assign_ports_client(struct ClientInfo *client);
void assign_ports_ss(struct StorageServerInfo *ss);
int compareFilePath(const char *X, const struct StorageServerInfo *array, int number_of_ss);
void createFileFromArray(const char *filename, struct CombinedFilesInfo *combinedFilesInfoAll, struct StorageServerInfo *storageServers, int number_of_ss);
void sendFile(int client_socket, const char *filename);

//* ss_utils.h
int createDirectory(const char *path, struct NM_to_SS_Response *response);
int createFilePath(const char *filePath, struct NM_to_SS_Response *response);
long get_file_size(FILE *fp);

//* print_utils.h
void print_response_info(struct Client_to_NM_response response);
void print_ss_info(struct StorageServerInfo *ss, struct CombinedFilesInfo combinedFilesInfo);
void print_client_info(struct ClientInfo client);
void print_client_request_info(struct ClientRequest client_request);
void print_client_request_info_ss(struct Client_to_SS_Request *request);
void logMessage(char* filepath,char *message);

//* data_utils.h
struct CombinedFilesInfo deserializeData(char *buffer, struct StorageServerInfo *ss);
void serializeData(struct StorageServerInfo *ss, int nFiles, int nDirectories, struct DirectoryInfo directories_all[], struct FileInfo files_all[], char *buffer);
char *get_substring_before_last_slash(const char *path);

//* network_wrapper.h
void close_socket(int socket);

//*errors.h
void print_error_message(int error_code);

#endif