#include "../include/common.h"

struct StorageServerInfo storage_server;
struct CombinedFilesInfo file_structure;
void *heartbeatThread(void *arg);
void *dataThread(void *arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronization

//* Global sockets
int socket_ss_initial;                            //* initiate_ss
int socket_server_heartbeat, socket_ss_heartbeat; //* Heartbeat Thread
int socket_server_nm, socket_ss_nm;               //* NamingServerConnection Thread

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

void handle_ctrl_c(int signum)
{
    printf("\nReceived Ctrl+C. Closing all sockets...\n");
    close_socket(socket_ss_initial);
    close_socket(socket_server_heartbeat);
    close_socket(socket_ss_heartbeat);
    close_socket(socket_server_nm);
    close_socket(socket_ss_nm);
    exit(0);
}

void print_ss_info(struct StorageServerInfo *ss)
{
    printf("IP Address: %s\n", ss->ipAddress);
    printf("Storage Server Port: %d\n", ss->storageServerPort);
    printf("Client Port: %d\n", ss->clientPort);
    printf("HeartBeat Port: %d\n", storage_server.heartbeatPort);
    printf("Is Connected: %d\n", ss->isConnected);
    printf("Storage Server ID: %d\n", ss->storageServerID);

    return;
}

//* Serialize the the StorageServerInfo, DirectoryInfo and FileInfo into a char buffer to send in a single send() call
void serializeData(struct StorageServerInfo *ss, int nFiles, int nDirectories, struct DirectoryInfo directories_all[], struct FileInfo files_all[], char *buffer)
{
    // print_ss_info(ss);
    memcpy(buffer, ss, sizeof(struct StorageServerInfo));
    // printf("Buffer: %s\n", buffer);
    // Calculate the offset for DirectoryInfo
    size_t offset = sizeof(struct StorageServerInfo);
    // printf("Offset: %d\n", offset);
    // Serialize DirectoryInfo
    memcpy(buffer + offset, directories_all, sizeof(struct DirectoryInfo) * nDirectories);
    // printf("Buffer: %s\n", buffer);
    // Calculate the new offset for FileInfo
    offset += sizeof(struct DirectoryInfo) * nDirectories;
    // printf("Offset: %d\n", offset);
    // Serialize FileInfo
    memcpy(buffer + offset, files_all, sizeof(struct FileInfo) * nFiles);

    // printf("Buffer: %s\n", buffer);
}

//* Initiate the Storage Server
void initiate_SS()
{
    struct sockaddr_in addr;

    socket_ss_initial = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_ss_initial < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LEADER_SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);

    if (connect(socket_ss_initial, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Error connecting to the leader server");
        close(socket_ss_initial);
        exit(EXIT_FAILURE);
    }

    //* Storage Server Info
    storage_server.storageServerID = 0;
    strncpy(storage_server.ipAddress, STORAGE_SERVER_IP, sizeof(storage_server.ipAddress));
    storage_server.storageServerPort = -1;
    storage_server.clientPort = -1;
    storage_server.heartbeatPort = -1;
    storage_server.isConnected = 1;

    //* Storage server directory info
    struct FileInfo *files = NULL;
    struct DirectoryInfo *directories = NULL;
    struct NumberOfFiles fileInfo;
    scan_dir(&files, &directories, &fileInfo);

    file_structure.files = files;
    file_structure.directories = directories;
    file_structure.numberOfFiles = fileInfo.numberOfFiles;
    file_structure.numberOfDirectories = fileInfo.numberOfDirectories;
    file_structure.storageServerID = 0;

    struct FileInfo files_all[file_structure.numberOfFiles];
    struct DirectoryInfo directories_all[file_structure.numberOfDirectories];

    for (int i = 0; i < file_structure.numberOfFiles; i++)
    {
        files_all[i] = file_structure.files[i];
        printf("File %d : %s\n", i + 1, files_all[i].name);
    }
    for (int i = 0; i < file_structure.numberOfDirectories; i++)
    {
        directories_all[i] = file_structure.directories[i];
        printf("Directory %d : %s\n", i + 1, directories_all[i].name);
    }

    storage_server.numberOfFiles = file_structure.numberOfFiles;
    storage_server.numberOfDirectories = file_structure.numberOfDirectories;

    char buffer[sizeof(struct StorageServerInfo) + sizeof(struct DirectoryInfo) * file_structure.numberOfDirectories + sizeof(struct FileInfo) * file_structure.numberOfFiles];
    serializeData(&storage_server, file_structure.numberOfFiles, file_structure.numberOfDirectories, directories_all, files_all, buffer);

    int bytes_sent;
    bytes_sent = send(socket_ss_initial, &buffer, sizeof(buffer), 0);

    if (bytes_sent == -1)
    {
        perror("Error sending data to leader server");
        close(socket_ss_initial);
        exit(EXIT_FAILURE);
    }

    //* will receive the storage server ID from Naming-Server
    int bytes_received;
    int ss_id;
    bytes_received = recv(socket_ss_initial, &storage_server, sizeof(storage_server), 0);
    if (bytes_received == -1)
    {
        perror("Error receiving data from leader server");
        close(socket_ss_initial);
        exit(EXIT_FAILURE);
    }
    else if (bytes_received == 0)
    {
        printf("Connection closed by the leader server.\n");
        close(socket_ss_initial);
        exit(EXIT_FAILURE);
    }

    storage_server.storageServerID = ss_id + 1;

    print_ss_info(&storage_server);

    close(socket_ss_initial);

    return;
}

//* Heartbeat thread for Storage-Server
void *heartbeatThread(void *arg)
{
    struct sockaddr_in address_server, address_ss;
    socklen_t size_address_client;

    socket_server_heartbeat = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_heartbeat < 0)
    {
        perror("[-]Socket error");
        close(socket_server_heartbeat);
        exit(1);
    }

    printf("[+]Storage-Server socket for HeartBeat created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(storage_server.heartbeatPort);
    address_server.sin_addr.s_addr = inet_addr(storage_server.ipAddress);
    size_address_client = sizeof(address_ss);

    if (bind(socket_server_heartbeat, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server_heartbeat);
        exit(1);
    }

    if (listen(socket_server_heartbeat, SERVER_BACKLOG) == 0)
    {
        printf("Storage-Server listening for HeartBeat from Naming-Server on PORT %d...\n", storage_server.heartbeatPort);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server_heartbeat);
        exit(1);
    }

    //* Will stop if Naming-Server disconnects
    while (1)
    {
        socket_ss_heartbeat = accept(socket_server_heartbeat, (struct sockaddr *)&address_ss, &size_address_client);
        if (socket_ss_heartbeat < 0)
        {
            perror("[-]Accepting error");
            close(socket_ss_heartbeat);
            continue;
        }
        printf("[+]Storage-Server connected with Naming-Server and listening for HeartBeat.\n");

        int bytes_received;
        int bytes_sent;

        struct StorageServerHeartBeat ss_heartbeat;
        while (1)
        {
            bytes_received = recv(socket_ss_heartbeat, &ss_heartbeat, sizeof(ss_heartbeat), 0);

            if (bytes_received == -1)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    printf("[-]Operation Would Block! Waiting for next recv().\n");
                }
                else if (errno == EINTR)
                {
                    printf("[-]recv() was interrupted by a signal. Try again.\n");
                }
                else
                {
                    perror("[-]recv() error");
                    close(socket_ss_heartbeat);
                    close(socket_server_heartbeat);
                    pthread_exit(NULL);
                    // break;
                }
            }
            else if (bytes_received == 0)
            {
                printf("Connection closed by the Naming-Server.\n");
                close(socket_server_heartbeat);
                close(socket_ss_heartbeat);
                pthread_exit(NULL);
                // break;
            }
            else
            {
                printf("[+]%s.\n", ss_heartbeat.buffer);

                bytes_sent = send(socket_ss_heartbeat, &ss_heartbeat, sizeof(ss_heartbeat), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending data to Naming-Server");
                    close(socket_server_heartbeat);
                    close(socket_ss_heartbeat);
                    pthread_exit(NULL);
                    // break;
                }
            }
        }

        printf("[+]Naming-Server disconnected.\n\n");
        printf("Storage-Server listening for HeartBeat from Naming-Server on PORT %d...\n", storage_server.heartbeatPort);
    }

    close(socket_server_heartbeat);
    pthread_exit(NULL);
}

void *namingServerConnectionThread(void *arg)
{
    struct sockaddr_in address_server, address_ss;
    socklen_t size_address_client;

    socket_server_nm = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_nm < 0)
    {
        perror("[-]Socket error");
        close(socket_server_nm);
        exit(1);
    }

    printf("[+]Storage-Server socket created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(storage_server.storageServerPort);
    address_server.sin_addr.s_addr = inet_addr(storage_server.ipAddress);
    size_address_client = sizeof(address_ss);

    if (bind(socket_server_nm, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server_nm);
        exit(1);
    }

    if (listen(socket_server_nm, SERVER_BACKLOG) == 0)
    {
        printf("Storage-Server listening for connection from Naming-Server on PORT %d...\n", storage_server.storageServerPort);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server_nm);
        exit(1);
    }

    while (1)
    {
        socket_ss_nm = accept(socket_server_nm, (struct sockaddr *)&address_ss, &size_address_client);
        if (socket_ss_nm < 0)
        {
            perror("[-]Accepting error");
            close(socket_ss_nm);
            continue;
        }
        printf("[+]Naming-Server connected.\n");

        int bytes_received;
        int bytes_sent;

        struct StorageServerHeartBeat ss_heartbeat;
        while (1)
        {

            bytes_received = recv(socket_ss_nm, &ss_heartbeat, sizeof(ss_heartbeat), 0);

            if (bytes_received == -1)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    printf("[-]Operation Would Block! Waiting for next recv().\n");
                }
                else if (errno == EINTR)
                {
                    printf("[-]recv() was interrupted by a signal. Try again.\n");
                }
                else
                {
                    perror("[-]recv() error");
                    close(socket_ss_nm);
                    break;
                }
            }
            else if (bytes_received == 0)
            {
                printf("Connection closed by the Naming-Server.\n");
                close(socket_ss_nm);
                break;
            }
            else
            {
                printf("[+]%s.\n", ss_heartbeat.buffer);

                bytes_sent = send(socket_ss_nm, &ss_heartbeat, sizeof(ss_heartbeat), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending data to Naming-Server");
                    close(socket_ss_nm);
                    break;
                }
            }
        }

        //? Should I close the thread?
        printf("[+]Naming-Server disconnected.\n\n");
        printf("Storage-Server listening for connection from Naming-Server on PORT %d...\n", storage_server.storageServerPort);
    }

    close(socket_server_nm);
    pthread_exit(NULL);
}

int main()
{
    signal(SIGINT, handle_ctrl_c);

    int ss_port_number, client_port_number, heartbeat_port_number;
    pthread_t heartbeat_thread, naming_server_connection;

    initiate_SS();
    pthread_create(&heartbeat_thread, NULL, heartbeatThread, NULL);
    pthread_create(&naming_server_connection, NULL, namingServerConnectionThread, NULL);

    pthread_join(naming_server_connection, NULL);
    pthread_join(heartbeat_thread, NULL);
    return 0;
}
