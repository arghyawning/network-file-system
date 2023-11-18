#include "../include/common.h"

int number_of_ss; //* Total number of SS attempted to connect
int number_of_connected_ss;
int ss_connection_status[MAX_SS] = {0};

int number_of_clients; //* Total number of clients attempted to connect
int number_of_connected_clients;
int client_connection_status[MAX_CLIENTS];

//* Client-Connection Threads
pthread_t clientThreads[MAX_CLIENTS];

//* Storage Server Heartbeat Threads
pthread_t storageServerThreads[MAX_SS];

//* Locks and conditional variables
pthread_mutex_t mutex_number_of_connected_ss = PTHREAD_MUTEX_INITIALIZER;

//* Storage Server Information
struct StorageServerInfo storageServers[MAX_SS];
struct CombinedFilesInfo combinedFilesInfoAll[MAX_SS];

//* Client Information
struct ClientInfo clients[MAX_CLIENTS];

int PORTS_SS[MAX_SS * MAX_PORTS_PER_SS];
int PORTS_CLIENTS[MAX_CLIENTS * MAX_PORTS_PER_SS];
//* function to assign ports to SS

int compareFilePath(const char *X, const struct StorageServerInfo *array)
{
    for (size_t i = 0; i < number_of_ss; i++)
    {

        if (strcmp(X, array[i].ss_boot_path) == 0)
        {
            // Match found
            return i; // Return the index where the match was found
        }
    }

    // No match found
    return -1;
}

void assign_ports_ss(struct StorageServerInfo *ss)
{
    int ss_id = ss->storageServerID;
    int base_port = BASE_PORT_SS + ss_id * MAX_PORTS_PER_SS;
    ss->heartbeatPort = base_port;
    ss->storageServerPort = base_port + 1;
    ss->clientPort = base_port + 2;

    return;
}

void assign_ports_client(struct ClientInfo *client)
{
    int client_id = client->clientID;
    int base_port = BASE_PORT_CLIENT + client_id * MAX_PORTS_PER_CLIENT;
    client->clientPort = base_port;

    return;
}

//* function to assign two random storage-servers for redundancy except the main storage-server. The storage-server should be active
void makeRedundantServers(struct StorageServerInfo *mainServer)
{
    printf("\n\nPreparing to assign Redundant-Servers for Storage-Server %d...\n", mainServer->storageServerID);
    // Seed the random number generator with the current time
    srand((unsigned int)time(NULL));
    int numRedundantServers = MAX_REDUNDANT_SS;
    // Count the number of active servers
    int activeServerCount = number_of_connected_ss;
    // for (int i = 0; i < MAX_SS; ++i)
    // {
    //     if (ss_connection_status[i] == 1)
    //     {
    //         activeServerCount++;
    //     }
    // }

    // Check if there are no active servers (except the main server)
    if (activeServerCount <= 1)
    {
        printf("No active storage servers to make redundant.\n");
        return;
    }

    // Determine the number of redundant servers to assign

    int indices_to_update[MAX_REDUNDANT_SS] = {-1};
    int count = 0;
    for (int i = 0; i < MAX_REDUNDANT_SS; i++)
    {
        if (mainServer->redundant_ss[i] == -1 || storageServers[mainServer->redundant_ss[i]].isConnected == 0)
        {
            indices_to_update[count] = i;
            count++;
        }
    }
    numRedundantServers = count;

    int actualNumRedundantServers = (activeServerCount - 1 < numRedundantServers) ? (activeServerCount - 1) : numRedundantServers;
    if (actualNumRedundantServers != numRedundantServers)
    {
        printf("Could not assign %d Redundant-Servers. Can allocate %d servers only.\n", numRedundantServers, actualNumRedundantServers);
    }

    // Randomly select redundant servers (apart from the main server)
    int redundantServers[MAX_SS] = {0};
    for (int i = 0; i < actualNumRedundantServers; i++)
    {
        if (indices_to_update[i] == -1)
        {
            redundantServers[mainServer->redundant_ss[indices_to_update[i]]] = 1;
        }
    }
    // for (int i = 0; i < actualNumRedundantServers; ++i)
    // {
    //     int redundantServer;
    //     while (1)
    //     {
    //         redundantServer = rand() % MAX_SS;
    //         if (redundantServer != mainServer->storageServerID && ss_connection_status[redundantServer] == 1 && !redundantServers[redundantServer])
    //         {
    //             redundantServers[redundantServer] = 1;
    //             break;
    //         }
    //     }

    //     // Print or use the redundant server as needed
    //     printf("Redundant Server %d: %d\n", i + 1, redundantServer);
    // }

    for (int i = 0; i < actualNumRedundantServers; ++i)
    {
        int redundantServer;
        while (1)
        {
            redundantServer = rand() % MAX_SS;
            if (redundantServer != mainServer->storageServerID && ss_connection_status[redundantServer] == 1 && !redundantServers[redundantServer])
            {
                redundantServers[redundantServer] = 1;
                mainServer->redundant_ss[indices_to_update[i]] = redundantServer;
                break;
            }
        }

        // Print or use the redundant server as needed
    }

    for (int i = 0; i < MAX_REDUNDANT_SS; i++)
    {
        printf("Redundant-Server : %d\n", mainServer->redundant_ss[i]);
    }
}

typedef struct file_found
{
    int ssid;                 // storage server id
    struct FileInfo filepath; // path of the file
} ff;

typedef struct per_key_bucket
{
    int key;             // length of filename
    ff files[MAX_FILES]; // files with same key
    int num_files;       // number of files with this particular key
} bucket;

// hash table for file search
bucket fileshash[MAX_PATH_SIZE + 1];

// hash function for file search
ff fileSearchWithHash(char *searchfilename)
{
    int key = strlen(searchfilename);
    bucket b = fileshash[key];

    for (int i = 0; i < b.num_files; i++)
    {
        // retrieving file name from path
        char *filename = strrchr(b.files[i].filepath.name, '/') + 1;
        if (filename == NULL)
            strcpy(filename, b.files[i].filepath.name);

        // comparing file name with searchfilename
        if (strcmp(filename, searchfilename) == 0)
            return b.files[i];
    }

    ff notfound;
    notfound.ssid = -1;
    notfound.filepath.name[0] = '\0';
    return notfound;
}

void initialize_hash_table()
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        fileshash[i].key = i;
        fileshash[i].num_files = 0;
    }
}

void store_in_hash(struct CombinedFilesInfo *files)
{
    printf("I was here 7\n");

    for (int i = 0; i < files->numberOfFiles; i++)
    {
        char *filename = strrchr(files->files[i].name, '/') + 1;

        printf("filename: %s\n", files->files[i].name);
        int key = strlen(filename);
        if (fileshash[key].num_files == MAX_FILES)
        {
            printf("[-]Hash table bucket full. Cannot store more files.\n");
            continue;
        }

        fileshash[key].files[fileshash[key].num_files] = (ff){files->storageServerID, files->files[i]};
        fileshash[key].num_files++;
        printf("A key : %d\nfilename: %s\n", key, files->files[i].name);
    }
    printf("I was here 8\n");
}

void print_hash_table()
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        bucket b = fileshash[i];
        printf("Key: %d\n", b.key);
        printf("Number of files: %d\n", b.num_files);
        for (int j = 0; j < b.num_files; j++)
        {
            printf("File %d: %s\n", j + 1, b.files[j].filepath.name);
        }
    }
}

void print_response_info(struct Client_to_NM_response response)
{
    printf("Transaction ID: %llu\n", response.transactionId);
    printf("Operation Performer: %d\n", response.operation_performer);
    printf("File Path: %s\n", response.file.name);
    if (response.operation_performer == 2)
    {
        printf("Storage Server Port: %d\n", response.ss_port);
        printf("Storage Server ID: %s\n", response.ss_ip);
    }
}

void print_ss_info(struct StorageServerInfo *ss)
{
    //* print ss info
    printf("Storage Server ID: %d\n", ss->storageServerID);
    printf("IP Address: %s\n", ss->ipAddress);
    printf("Storage Server Port: %d\n", ss->storageServerPort);
    printf("Client Port: %d\n", ss->clientPort);
    printf("Number of Files: %d\n", ss->numberOfFiles);
    printf("Number of Directories: %d\n", ss->numberOfDirectories);
    printf("Storage-Server boot path: %s\n", ss->ss_boot_path);
    printf("Is Connected: %d\n", ss->isConnected);

    //* print files and directory information
    for (int i = 0; i < ss->numberOfFiles; i++)
    {
        printf("File %d: %s\n", i + 1, combinedFilesInfoAll[ss->storageServerID].files[i].name);
    }
    for (int i = 0; i < ss->numberOfDirectories; i++)
    {
        printf("Directory %d: %s\n", i + 1, combinedFilesInfoAll[ss->storageServerID].directories[i].name);
    }

    return;
}

void print_client_info(struct ClientInfo client)
{
    printf("Client ID: %d\n", client.clientID);
    printf("Client Name: %s\n", client.clientName);
    printf("Session ID: %d\n", client.sessionID);
    printf("IP Address: %s\n", client.ipAddress);
    printf("Client Port: %d\n", client.clientPort);
    printf("Is Connected: %d\n", client.isConnected);
}

void print_client_request_info(struct ClientRequest client_request)
{
    printf("Client ID: %d\n", client_request.clientID);
    printf("Transaction ID: %llu\n", client_request.transactionId);
    printf("Operation: %s\n", client_request.command);
    printf("Argument 1: %s\n", client_request.arguments[0]);
    printf("Argument 2: %s\n", client_request.arguments[1]);
}

//* Deserialize the char buffer into StorageServerInfo, DirectoryInfo and FileInfo
struct CombinedFilesInfo deserializeData(char *buffer, struct StorageServerInfo *ss)
{
    int nFiles, nDirectories;

    //* Deserialize StorageServerInfo
    memcpy(ss, buffer, sizeof(struct StorageServerInfo));
    nFiles = ss->numberOfFiles;
    nDirectories = ss->numberOfDirectories;

    //* Calculate the offset for DirectoryInfo
    size_t offset = sizeof(struct StorageServerInfo);

    //* Deserialize DirectoryInfo
    struct DirectoryInfo *directories_all = (struct DirectoryInfo *)malloc(sizeof(struct DirectoryInfo) * nDirectories);
    memcpy(directories_all, buffer + offset, sizeof(struct DirectoryInfo) * nDirectories);

    // *Calculate the new offset for FileInfo
    offset += sizeof(struct DirectoryInfo) * nDirectories;

    //* Deserialize FileInfo
    struct FileInfo *files_all = (struct FileInfo *)malloc(sizeof(struct FileInfo) * nFiles);
    memcpy(files_all, buffer + offset, sizeof(struct FileInfo) * nFiles);

    struct CombinedFilesInfo combinedFilesInfo;
    combinedFilesInfo.numberOfFiles = ss->numberOfFiles;
    combinedFilesInfo.numberOfDirectories = ss->numberOfDirectories;
    combinedFilesInfo.files = files_all;
    combinedFilesInfo.directories = directories_all;

    return combinedFilesInfo;
}

// void *checkRedundantServers(void *arg)
// {

//     while (1)
//     {
//         sleep(1);

//         pthread_mutex_lock(&mutex_number_of_connected_ss);

//         for (int i = 0; i < number_of_ss; i++)
//         {
//             if (storageServers[i].isConnected == 1)
//             {
//                 for (int j = 0; j < MAX_REDUNDANT_SS; j++)
//                 {
//                 }
//             }
//         }
//         // Check if main server and its redundant servers are active
//         if (ss_connection_status[mainServer->serverID] == 0 || number_of_active_ss < 2)
//         {
//             // Main server or one of the redundant servers is not active
//             printf("Main server or redundant servers for Server %d are not active.\n", mainServer->serverID);
//         }

//         pthread_mutex_unlock(&mutex_number_of_connected_ss);
//     }

//     return NULL;
// }

void *client_connection(void *arg)
{
    struct ClientInfo *client = (struct ClientInfo *)arg;
    int client_id = client->clientID;
    printf("[+]Client %d thread created.\n", client_id + 1);

    int socket_server_nm, socket_client;
    struct sockaddr_in address_server, address_ss;
    socklen_t size_address_client;

    socket_server_nm = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_nm < 0)
    {
        perror("[-]Socket error");
        close(socket_server_nm);
        exit(1);
    }

    printf("[+]Naming-Server socket created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(client->clientPort);
    address_server.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);
    size_address_client = sizeof(address_ss);

    if (bind(socket_server_nm, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server_nm);
        exit(1);
    }

    if (listen(socket_server_nm, SERVER_BACKLOG) == 0)
    {
        printf("Naming-Server listening for connection from Client on PORT %d...\n", client->clientPort);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server_nm);
        exit(1);
    }

    while (1)
    {
        socket_client = accept(socket_server_nm, (struct sockaddr *)&address_ss, &size_address_client);
        if (socket_client < 0)
        {
            perror("[-]Accepting error");
            close(socket_client);
            continue;
        }
        printf("[+]Client connected.\n");

        int bytes_received;
        int bytes_sent;

        struct ClientRequest client_request;
        struct Client_to_NM_response response;
        while (1)
        {

            bytes_received = recv(socket_client, &client_request, sizeof(client_request), 0);

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
                    close(socket_client);
                    close(socket_server_nm);
                    pthread_exit(NULL);
                }
            }
            else if (bytes_received == 0)
            {
                printf("Connection closed by the Client.\n");
                close(socket_client);
                close(socket_server_nm);
                pthread_exit(NULL);
            }
            else
            {
                print_client_request_info(client_request);
                response.transactionId = client_request.transactionId;

                if (strcmp(client_request.command, "READ") == 0 || strcmp(client_request.command, "FILEINFO") == 0 || strcmp(client_request.command, "WRITE") == 0)
                {
                    ff file;
                    file.ssid = -1;
                    file = fileSearchWithHash(client_request.arguments[0]);
                    printf("File found on storage server %d\n", file.ssid);
                    printf("file path : %s\n", file.filepath.name);

                    response.operation_performer = 2;
                    response.ss_port = storageServers[file.ssid].clientPort;
                    strcpy(response.ss_ip, storageServers[file.ssid].ipAddress);
                    strcpy(response.file.name, file.filepath.name);
                }
                else if (strcmp(client_request.command, "CREATE") == 0 || strcmp(client_request.command, "DELETE") == 0 || strcmp(client_request.command, "COPY") == 0)
                {
                    ff file;
                    file.ssid = -1;
                    file = fileSearchWithHash(client_request.arguments[0]);
                    printf("File found on storage server %d\n", file.ssid);
                    printf("file path : %s\n", file.filepath.name);

                    strcpy(response.file.name, file.filepath.name);
                    response.operation_performer = 1;
                }

                print_response_info(response);

                bytes_sent = send(socket_client, &response, sizeof(response), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending data to Client");
                    close(socket_client);
                    close(socket_server_nm);
                    pthread_exit(NULL);
                }
            }
        }

        //? Should I close the thread?
        printf("[+]Client disconnected.\n\n");
        printf("Naming-Server listening for connection from Client on PORT %d...\n", client->clientPort);
    }

    close(socket_server_nm);
    pthread_exit(NULL);
}

//* Heartbeat thread for each storage server
void *storage_server_heartbeat(void *arg)
{
    //* ss_id is the index of the storage server in the storageServers array
    struct storage_server_connection_struct *temp = (struct storage_server_connection_struct *)arg;
    int ss_id = temp->ss_id;
    printf("[+]Storage-Server %d thread created.\n", ss_id + 1);
    int socket_ss;
    struct sockaddr_in addr;
    socklen_t size_address;

    socket_ss = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ss < 0)
    {
        perror("[-]Socket error");
        close(socket_ss);
        exit(1);
    }

    printf("[+]Storage-Server %d socket created.\n", ss_id + 1);

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(storageServers[ss_id].heartbeatPort);
    addr.sin_addr.s_addr = inet_addr(storageServers[ss_id].ipAddress);

    size_address = sizeof(addr);

    int connection_tries = 0;
    while (1)
    {
        if (connect(socket_ss, (struct sockaddr *)&addr, size_address) == 0)
        {
            printf("[+]Connected to Storage-Server %d.\n", ss_id + 1);
            ss_connection_status[ss_id] = 1;
            storageServers[ss_id].isConnected = 1;
            break;
        }
        else
        {
            printf("[-]Connection attempt %d/%d to Storage-Server %d on PORT %d failed. Trying again...\n", connection_tries + 1, MAX_CONNECTION_TRIES, ss_id + 1, storageServers[ss_id].heartbeatPort);
            connection_tries++;
            sleep(RETRY_INTERVAL);
        }

        if (connection_tries == MAX_CONNECTION_TRIES)
        {
            printf("[-]Could not connect to Storage-Server %d. Stopping further connection attempts\n", ss_id + 1);
            close(socket_ss);
            pthread_exit(NULL);
        }
    }

    pthread_mutex_lock(&mutex_number_of_connected_ss);
    number_of_connected_ss++;
    printf("\n\nNumber of connected storage servers: %d\n\n", number_of_connected_ss);
    pthread_mutex_unlock(&mutex_number_of_connected_ss);

    makeRedundantServers(&storageServers[ss_id]);

    int ticks = 0;
    while (1)
    {
        int bytes_sent, bytes_received;
        struct StorageServerHeartBeat ss_heartbeat;
        sprintf(ss_heartbeat.buffer, "Heartbeat %d from Naming-Server to Storage-Server %d", ticks, ss_id + 1);
        ss_heartbeat.heartbeatCount = ticks;
        ss_heartbeat.ss_id = ss_id + 1;
        bytes_sent = send(socket_ss, &ss_heartbeat, sizeof(ss_heartbeat), 0);

        if (bytes_sent == -1)
        {
            perror("Error sending heartbeat to storage server");
            close(socket_ss);
            ss_connection_status[ss_id] = 0;
            storageServers[ss_id].isConnected = 0;
            printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        bytes_received = recv(socket_ss, &ss_heartbeat, sizeof(ss_heartbeat), 0);
        if (bytes_received == -1)
        {
            perror("Error receiving heartbeat from storage server");
            close(socket_ss);
            ss_connection_status[ss_id] = 0;
            storageServers[ss_id].isConnected = 0;
            printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Storage-Server.\n");
            close(socket_ss);
            ss_connection_status[ss_id] = 0;
            storageServers[ss_id].isConnected = 0;
            printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        ticks++;
        sleep(HEARTBEAT_PERIOD);
    }
    pthread_mutex_lock(&mutex_number_of_connected_ss);
    number_of_connected_ss--;
    printf("\n\nNumber of connected storage servers: %d\n\n", number_of_connected_ss);
    pthread_mutex_unlock(&mutex_number_of_connected_ss);
    pthread_exit(NULL);
}

//* Thread to listen for connections from storage servers
void *first_connection_ss(void *arg)
{
    number_of_ss = 0;
    int socket_server, socket_ss;
    struct sockaddr_in address_server, address_ss;
    socklen_t size_address_client;

    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server < 0)
    {
        perror("[-]Socket error");
        close(socket_server);
        exit(1);
    }

    printf("[+]Naming-Server socket created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(LEADER_SERVER_PORT);
    address_server.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);
    size_address_client = sizeof(address_ss);

    if (bind(socket_server, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server);
        exit(1);
    }

    if (listen(socket_server, SERVER_BACKLOG) == 0)
    {
        printf("Naming-Server listening for Storage-Server connections on PORT %d...\n", LEADER_SERVER_PORT);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server);
        exit(1);
    }

    while (1)
    {
        socket_ss = accept(socket_server, (struct sockaddr *)&address_ss, &size_address_client);
        if (socket_ss < 0)
        {
            perror("[-]Accepting error");
            close(socket_ss);
            continue;
        }
        printf("[+]Storage-Server connected.\n");

        int bytes_received;
        int bytes_sent;
        struct StorageServerInfo ss;
        struct CombinedFilesInfo combinedFilesInfo;
        struct DirectoryInfo *directories = NULL;
        struct FileInfo *files = NULL;
        char buffer[SERIALIZED_BUFFER_SIZE];
        bytes_received = recv(socket_ss, &buffer, sizeof(buffer), 0);
        if (bytes_received == -1)
        { //* File Name with relative path to the directory where storage server was started from
            struct FileInfo
            {
                char name[MAX_PATH_SIZE];
            };

            //* Directory Name with relative path to the directory where storage server was started from
            struct DirectoryInfo
            {
                char name[MAX_PATH_SIZE];
            };
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
                close(socket_ss);
                break;
            }
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Storage-Server.\n");
            close(socket_ss);
            break;
        }
        else
        {

            //! Do not change this order
            combinedFilesInfo = deserializeData(buffer, &ss);
            files = combinedFilesInfo.files;
            directories = combinedFilesInfo.directories;

            int server_exists = compareFilePath(ss.ss_boot_path, storageServers);
            printf("I was here 1\n");
            if (server_exists == -1)
            {
                ss.storageServerID = number_of_ss;
                combinedFilesInfo.storageServerID = ss.storageServerID;
                assign_ports_ss(&ss);
                printf("I was here 2\n");

                combinedFilesInfoAll[number_of_ss] = combinedFilesInfo;
                storageServers[number_of_ss] = ss;
                printf("I was here 3\n");

                printf("I was here 4\n");
                store_in_hash(&combinedFilesInfoAll[number_of_ss]);
                printf("I was here 5\n");

                //* send updated storage-server
                bytes_sent = send(socket_ss, &ss, sizeof(ss), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending SS-ID to storage server");
                    close(socket_ss);
                    break;
                }
                printf("I was here 6\n");

                print_ss_info(&ss);
                struct storage_server_connection_struct ss_struct;
                ss_struct.ss_id = number_of_ss;
                pthread_create(&storageServerThreads[number_of_ss], NULL, storage_server_heartbeat, (void *)&ss_struct);

                number_of_ss++;
            }
            else
            {

                printf("Storage server already exists\n");
                //* send updated storage-server
                bytes_sent = send(socket_ss, &storageServers[server_exists], sizeof(storageServers[server_exists]), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending SS-ID to storage server");
                    close(socket_ss);
                    break;
                }

                print_ss_info(&storageServers[server_exists]);
                struct storage_server_connection_struct ss_struct;
                ss_struct.ss_id = server_exists;
                pthread_create(&storageServerThreads[server_exists], NULL, storage_server_heartbeat, (void *)&ss_struct);
            }
        }

        printf("[+]Current Storage-Server disconnected.\n\n");
        printf("Naming-Server listening for Storage-Server connections on PORT %d...\n", LEADER_SERVER_PORT);
    }

    for (int i = 0; i < number_of_ss; i++)
    {
        if (ss_connection_status[i] == 1)
        {
            pthread_join(storageServerThreads[i], NULL);
        }
    }

    close(socket_server);

    //* Close the thread
    pthread_exit(NULL);
}

void *first_connection_client(void *arg)
{
    number_of_clients = 0;
    int socket_server, socket_client;
    struct sockaddr_in address_server, address_client;
    socklen_t size_address_client;

    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server < 0)
    {
        perror("[-]Socket error");
        close(socket_server);
        exit(1);
    }

    printf("[+]Naming-Server socket created.\n");

    memset(&address_server, '\0', sizeof(address_server));
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(LEADER_SERVER_PORT_CLIENT);
    address_server.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);
    size_address_client = sizeof(address_client);

    if (bind(socket_server, (struct sockaddr *)&address_server, sizeof(address_server)) < 0)
    {
        perror("[-]Bind error");
        close(socket_server);
        exit(1);
    }

    if (listen(socket_server, SERVER_BACKLOG) == 0)
    {
        printf("Naming-Server listening for Client connections on PORT %d...\n", LEADER_SERVER_PORT_CLIENT);
    }
    else
    {
        perror("[-]Listening error");
        close(socket_server);
        exit(1);
    }

    while (1)
    {
        socket_client = accept(socket_server, (struct sockaddr *)&address_client, &size_address_client);
        if (socket_client < 0)
        {
            perror("[-]Accepting error");
            close(socket_client);
            continue;
        }
        printf("[+] Client connected.\n");

        int bytes_received;
        int bytes_sent;
        struct ClientInfo client;

        bytes_received = recv(socket_client, &client, sizeof(client), 0);
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
                close(socket_client);
            }
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Client.\n");
            close(socket_client);
        }
        else
        {
            client.clientID = number_of_clients;
            client.sessionID = 0;
            assign_ports_client(&client);

            clients[number_of_clients] = client;

            print_client_info(client);
            bytes_sent = send(socket_client, &client, sizeof(client), 0);
            if (bytes_sent == -1)
            {
                perror("Error sending Client-ID to Client");
                close(socket_client);
            }

            pthread_create(&clientThreads[number_of_clients], NULL, client_connection, (void *)&client);

            number_of_clients++;
        }
    }
}

int main()
{
    number_of_connected_ss = 0;
    pthread_t first_connection_client_thread, first_connection_ss_thread;
    initialize_hash_table();
    //* thread for listening to first connection from storage server
    pthread_create(&first_connection_ss_thread, NULL, first_connection_ss, NULL);
    pthread_create(&first_connection_client_thread, NULL, first_connection_client, NULL);

    pthread_join(first_connection_ss_thread, NULL);
    pthread_join(first_connection_client_thread, NULL);
    return 0;
}
