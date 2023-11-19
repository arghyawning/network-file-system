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
int socket_server_cc, socket_ss_cc;               //* ClientConnection Thread

void removeDirectoryRecursive(const char *path)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL)
    {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char fullPath[MAX_PATH_SIZE];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

            struct stat statbuf;
            if (stat(fullPath, &statbuf) == -1)
            {
                perror("Error getting file/directory information");
                closedir(dir);
                return;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                removeDirectoryRecursive(fullPath); // Recursive call for subdirectory
            }
            else
            {
                if (remove(fullPath) != 0)
                {
                    perror("Error removing file");
                    closedir(dir);
                    return;
                }
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0)
    {
        perror("Error removing directory");
    }
}

void readFile(char *filepath, int socket)
{
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        perror("[-] Error in reading file.");
        exit(1);
    }

    struct Packet packet;
    packet.packetNumber = 0; // Initialize packet number
    packet.isStart = 1;
    packet.isEnd = 0;

    while (fgets(packet.data, READ_SIZE, fp) != NULL)
    {
        packet.packetNumber++;

        if (send(socket, &packet, sizeof(struct Packet), 0) == -1)
        {
            perror("[-] Error in sending data");
            exit(1);
        }
        printf("[+]Packet %d sent.\n", packet.packetNumber);
    }

    // Send an end-of-file packet
    printf("[+]Sending end-of-file packet.\n");
    packet.packetNumber++;
    packet.isStart = 0;
    packet.isEnd = 1;
    if (send(socket, &packet, sizeof(struct Packet), 0) == -1)
    {
        perror("[-] Error in sending data");
        exit(1);
    }

    fclose(fp);
}

void readFileInfo(char *filepath, int socket)
{
    struct stat fileStat;
    if (stat(filepath, &fileStat) < 0)
    {
        printf("Cannot get file information \n");
        return;
    }
    int bytes_sent;
    bytes_sent = send(socket, &fileStat, sizeof(fileStat), 0);

    if (bytes_sent == -1)
    {
        perror("Error sending data to client");
        close(socket);
        exit(EXIT_FAILURE);
    }

    return;
}

void writeToFile(char *filepath, int write_type, int socket)
{

    FILE *file;
    if (write_type == 1)
    {
        file = fopen(filepath, "w");
    }
    else if (write_type == 2)
    {
        file = fopen(filepath, "a");
    }

    if (file == NULL)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    struct PacketWrite packet;
    int total_bytes_received = 0;

    while (1)
    {
        // Receive the packet
        ssize_t bytes_received = recv(socket, &packet, sizeof(struct PacketWrite), 0);

        if (bytes_received == -1)
        {
            perror("Error receiving data");
            break;
        }
        else if (bytes_received == 0)
        {
            // Connection closed by the client
            break;
        }

        // Write the data to the file
        // fwrite(packet.data, 1, strlen(packet.data), file);
        fprintf(file, "%s", packet.data);
        printf("[+]Packet %d received.\n", packet.packetNumber);
        printf("[+]Data: %s\n", packet.data);
        printf("[+]Data length: %ld\n", strlen(packet.data));
        total_bytes_received += strlen(packet.data);

        // Check for end of file
        if (packet.isEnd)
        {
            // char null_char = '\0';
            // fwrite(&null_char, 1, 1, file);
            printf("End of file reached. Total bytes received: %d\n", total_bytes_received);
            break;
        }
    }

    fclose(file);
}

void handle_ctrl_c(int signum)
{
    printf("\nReceived Ctrl+C. Closing all sockets...\n");
    close_socket(socket_ss_initial);
    close_socket(socket_server_heartbeat);
    close_socket(socket_ss_heartbeat);
    close_socket(socket_server_nm);
    close_socket(socket_ss_nm);
    close_socket(socket_server_cc);
    close_socket(socket_ss_cc);
    exit(0);
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

    for (int i = 0; i < MAX_REDUNDANT_SS; i++)
    {
        storage_server.redundant_ss[i] = -1;
    }
    storage_server.numberOfFiles = file_structure.numberOfFiles;
    storage_server.numberOfDirectories = file_structure.numberOfDirectories;
    getcwd(storage_server.ss_boot_path, sizeof(storage_server.ss_boot_path));
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
    int ss_id = -1;
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

    struct CombinedFilesInfo combinedFilesInfo;
    // combinedFilesInfo = deserializeData(buffer, &storage_server);
    // print_ss_info(&storage_server, combinedFilesInfo);

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

        struct ClientRequest request;

        bytes_received = recv(socket_ss_nm, &request, sizeof(request), 0);

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
            printf("[+]%s.\n", request.command);
            if (strcmp(request.command, "CREATE") == 0)
            {
                if (strcmp(request.arguments[0], "FILE") == 0)
                {
                    printf("Creating file...\n");
                    FILE *fp = fopen(request.arguments[1], "w");
                    if (fp == NULL)
                    {
                        perror("[-] Error in creating file.");
                        exit(1);
                    }
                    fclose(fp);
                    printf("File created.\n");
                }
                else if (strcmp(request.arguments[0], "DIR") == 0)
                {
                    printf("Creating directory...\n");
                    if (mkdir(request.arguments[1], 0777) == -1)
                    {
                        perror("[-] Error in creating directory.");
                        exit(1);
                    }
                    printf("Directory created.\n");
                }
            }
            else if (strcmp(request.command, "DELETE") == 0)
            {
                if (strcmp(request.arguments[0], "FILE") == 0)
                {
                    printf("Deleting file...\n");
                    if (remove(request.arguments[1]) == 0)
                    {
                        printf("File '%s' removed successfully.\n", request.arguments[1]);
                    }
                    else
                    {
                        perror("Error removing file");
                    }
                    printf("File removed.\n");
                }
                else if (strcmp(request.arguments[0], "DIR") == 0)
                {
                    printf("Deleting directory...\n");
                    removeDirectoryRecursive(request.arguments[1]);
                    printf("Directory created.\n");
                }
            }
            // else if (strcmp(request.command, "COPY") == 0)
            // {
            //     printf("Copying file...\n");
            //     FILE *fptr1, *fptr2;
            //     char c;
            //     fptr1 = fopen(request.file.name, "r");
            //     if (fptr1 == NULL)
            //     {
            //         printf("Cannot open file %s \n", request.file.name);
            //         exit(0);
            //     }
            //     fptr2 = fopen(request.buffer, "w");
            //     if (fptr2 == NULL)
            //     {
            //         printf("Cannot open file %s \n", request.buffer);
            //         exit(0);
            //     }
            //     c = fgetc(fptr1);
            //     while (c != EOF)
            //     {
            //         fputc(c, fptr2);
            //         c = fgetc(fptr1);
            //     }
            //     printf("File copied successfully.\n");
            //     fclose(fptr1);
            //     fclose(fptr2);
            // }

            struct NM_to_SS_Response response;
            response.operation_status = 1;
            bytes_sent = send(socket_ss_nm, &response, sizeof(response), 0);
            if (bytes_sent == -1)
            {
                perror("Error sending data to Naming-Server");
                close(socket_ss_nm);
                break;
            }
        }

        //? Should I close the thread?
        printf("[+]Naming-Server disconnected.\n\n");
        printf("Storage-Server listening for connection from Naming-Server on PORT %d...\n", storage_server.storageServerPort);
    }

    close(socket_server_nm);
    pthread_exit(NULL);
}

void *clientConnectionThread(void *arg)
{
    struct sockaddr_in address_server, address_ss;
    socklen_t size_address_client;

    socket_server_cc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_cc < 0)
    {
        perror("[-]Socket error");
        close(socket_server_cc);
        exit(1);
    }

    printf("[+]Storage-Server socket created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(storage_server.clientPort);
    address_server.sin_addr.s_addr = inet_addr(storage_server.ipAddress);
    size_address_client = sizeof(address_ss);

    if (bind(socket_server_cc, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server_cc);
        exit(1);
    }

    if (listen(socket_server_cc, SERVER_BACKLOG) == 0)
    {
        printf("Storage-Server listening for connection from Client on PORT %d...\n", storage_server.clientPort);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server_cc);
        exit(1);
    }

    while (1)
    {
        socket_ss_cc = accept(socket_server_cc, (struct sockaddr *)&address_ss, &size_address_client);
        if (socket_ss_cc < 0)
        {
            perror("[-]Accepting error");
            close(socket_ss_cc);
            continue;
        }
        printf("[+]Client connected.\n");

        int bytes_received;
        int bytes_sent;

        struct Client_to_SS_Request request;

        bytes_received = recv(socket_ss_cc, &request, sizeof(request), 0);

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
                close(socket_ss_cc);
                break;
            }
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Naming-Server.\n");
            close(socket_ss_cc);
            break;
        }
        else
        {

            print_client_request_info_ss(&request);

            //* READ
            if (strcmp(request.command, "READ") == 0)
            {
                //* Check if file exists
                // int file_exists = 0;
                // for (int i = 0; i < file_structure.numberOfFiles; i++)
                // {
                //     if (strcmp(request.file.name, file_structure.files[i].name) == 0)
                //     {
                //         file_exists = 1;
                //         break;
                //     }
                // }

                // if (file_exists == 0)
                // {
                //     printf("File does not exist.\n");
                //     continue;
                // }

                //* Send file to client
                printf("Sending file to client...\n");
                readFile(request.file.name, socket_ss_cc);
                printf("File sent to client.\n");
            }
            else if (strcmp(request.command, "FILEINFO") == 0)
            {
                // int file_exists = 0;
                // for (int i = 0; i < file_structure.numberOfFiles; i++)
                // {
                //     if (strcmp(request.file.name, file_structure.files[i].name) == 0)
                //     {
                //         file_exists = 1;
                //         break;
                //     }
                // }
                // if (file_exists == 0)
                // {
                //     printf("File does not exist.\n");
                //     continue;
                // }

                //* Send file to client
                printf("Sending file information to client...\n");
                readFileInfo(request.file.name, socket_ss_cc);
                printf("File Information sent to client.\n");
            }
            else if (strcmp(request.command, "WRITE") == 0)
            {
                // int file_exists = 0;
                // for (int i = 0; i < file_structure.numberOfFiles; i++)
                // {
                //     if (strcmp(request.file.name, file_structure.files[i].name) == 0)
                //     {
                //         file_exists = 1;
                //         break;
                //     }
                // }
                // if (file_exists == 0)
                // {
                //     printf("File does not exist.\n");
                //     continue;
                // }

                //* Send file to client
                printf("Starting Write Operation...\n");
                writeToFile(request.file.name, request.write_type, socket_ss_cc);
                printf("File Written to.\n");
            }
        }
        close(socket_ss_cc);
        //? Should I close the thread?
        printf("[+]Client disconnected.\n\n");
        printf("Storage-Server listening for connection from Client on PORT %d...\n", storage_server.clientPort);
    }

    close(socket_server_cc);
    pthread_exit(NULL);
}

int main()
{
    signal(SIGINT, handle_ctrl_c);

    pthread_t heartbeat_thread, naming_server_connection, client_connection;

    initiate_SS();
    pthread_create(&heartbeat_thread, NULL, heartbeatThread, NULL);
    pthread_create(&naming_server_connection, NULL, namingServerConnectionThread, NULL);
    pthread_create(&client_connection, NULL, clientConnectionThread, NULL);

    pthread_join(client_connection, NULL);
    pthread_join(naming_server_connection, NULL);
    pthread_join(heartbeat_thread, NULL);
    return 0;
}
