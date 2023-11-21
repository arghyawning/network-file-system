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

//* Hash table for file search
bucket fileshash[MAX_PATH_SIZE + 1];
bucket_dir dirhash[MAX_PATH_SIZE + 1];

//* Ports for SS and Client
int PORTS_SS[MAX_SS * MAX_PORTS_PER_SS];
int PORTS_CLIENTS[MAX_CLIENTS * MAX_PORTS_PER_SS];

//* Loggin file
char logFilePath[MAX_PATH_SIZE] = ".nm_log";

//* Search Tree
struct DirTree dirTree;

void printTree(struct DirTree *dirTree, int level)
{
    if (dirTree == NULL)
        return;

    for (int i = 0; i < level; i++)
        printf("\t");

    printf("%s (ssid %d)\n", dirTree->dirinfo.name, dirTree->ssid);

    for (int i = 0; i < dirTree->numberOfDirectories; i++)
    {
        printTree(&dirTree->directories[i], level + 1);
    }
}

//* function to assign two random storage-servers for redundancy except the main storage-server. The storage-server should be active
void makeRedundantServers(struct StorageServerInfo *mainServer)
{
    // Log the message
    char temp[MAX_BUFFER_SIZE];
    sprintf(temp, "Preparing to assign Redundant-Servers for Storage-Server %d...\n", mainServer->storageServerID);
    logMessage(logFilePath, temp);
    printf(temp);
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

void prepare_for_copy(struct ClientRequest *request)
{
    //* source_path will be of the form "./a/b/c.txt" and destination path will be of the form "./c/d"
    //* We need to extract the file name from the source path and append it to the destination path

    // char *source_path = strrchr(request->arguments[0], '/') + 1;
    char *source_path = request->arguments[0];
    char *destination_path = get_substring_before_last_slash(request->arguments[1]);
    // destination_path = strrchr(destination_path, '/') + 1;

    // printf("Modified Source path: %s\n", source_path);
    // printf("Modified Destination path: %s\n", destination_path);

    int source_storage_ss = fileSearchWithHash(source_path, fileshash).ssid;
    int destination_ss = dirSearchWithHash(destination_path, dirhash).ssid;

    int source_storage_port = BASE_PORT_SS + source_storage_ss * MAX_PORTS_PER_SS + 3;
    int destination_port = BASE_PORT_SS + destination_ss * MAX_PORTS_PER_SS + 3;

    request->copyInfo.ss_source_id = source_storage_ss;
    request->copyInfo.ss_destination_id = destination_ss;
    request->copyInfo.ss_source_port = source_storage_port;
    request->copyInfo.ss_destination_port = destination_port;

    printf("Source storage server: %d\n", request->copyInfo.ss_source_id);
    printf("Destination storage server: %d\n", request->copyInfo.ss_destination_id);
    printf("Source storage server port: %d\n", request->copyInfo.ss_source_port);
    printf("Destination storage server port: %d\n", request->copyInfo.ss_destination_port);

    return;
}

void copy_file_between_ss(struct ClientRequest request)
{
    struct CopyInfo copyInfo;
    prepare_for_copy(&request);
    copyInfo = request.copyInfo;

    int storage_server1, storage_server2, main_server;
    struct sockaddr_in storage1_addr, storage2_addr, main_addr;

    // Create socket for storage server 1
    storage_server1 = socket(AF_INET, SOCK_STREAM, 0);
    if (storage_server1 == -1)
    {
        perror("Error creating socket for storage server 1");
        exit(EXIT_FAILURE);
    }

    // Set up address for storage server 1
    memset(&storage1_addr, 0, sizeof(storage1_addr));
    storage1_addr.sin_family = AF_INET;
    storage1_addr.sin_addr.s_addr = inet_addr(storageServers[copyInfo.ss_source_id].ipAddress);
    storage1_addr.sin_port = htons(storageServers[copyInfo.ss_source_id].storageServerPort); // Replace with the actual port number

    // Connect to storage server 1
    if (connect(storage_server1, (struct sockaddr *)&storage1_addr, sizeof(storage1_addr)) == -1)
    {
        perror("Error connecting to storage server 1");
        close(storage_server1);
        exit(EXIT_FAILURE);
    }

    char tempLog[MAX_BUFFER_SIZE];
    sprintf(tempLog, "Connected to storage server %d for copying.\n", copyInfo.ss_source_id);
    logMessage(logFilePath, tempLog);
    printf("Connected to source storage server %d for copying. \n", copyInfo.ss_source_id);

    // Repeat the above steps for storage server 2 and main server

    // Create socket for storage server 2
    storage_server2 = socket(AF_INET, SOCK_STREAM, 0);
    if (storage_server2 == -1)
    {
        perror("Error creating socket for storage server 2");
        logMessage(logFilePath, "Error creating socket for storage server 2");
        close(storage_server1);
        return;
    }

    memset(&storage2_addr, 0, sizeof(storage2_addr));
    storage2_addr.sin_family = AF_INET;
    storage2_addr.sin_addr.s_addr = inet_addr(storageServers[copyInfo.ss_destination_id].ipAddress);
    storage2_addr.sin_port = htons(storageServers[copyInfo.ss_destination_id].storageServerPort); // Replace with the actual port number

    if (connect(storage_server2, (struct sockaddr *)&storage2_addr, sizeof(storage2_addr)) == -1)
    {
        perror("Error connecting to storage server 2");
        logMessage(logFilePath, "Error connecting to storage server 2");
        close(storage_server1);
        close(storage_server2);
        return;
    }

    sprintf(tempLog, "Connected to storage server %d for copying.\n", copyInfo.ss_destination_id);
    printf("Connected to destination storage server %d for copying. \n", copyInfo.ss_destination_id);
    logMessage(logFilePath, tempLog);

    int bytes_sent, bytes_received;
    struct NM_to_SS_Response source_response, destination_response;
    bytes_sent = send(storage_server1, &request, sizeof(struct ClientRequest), 0);
    if (bytes_sent == -1)
    {
        perror("Error sending request to storage server 1");
        logMessage(logFilePath, "Error sending request to storage server 1");
        close(storage_server1);
        close(storage_server2);
        return;
    }
    bytes_received = recv(storage_server1, &source_response, sizeof(struct NM_to_SS_Response), 0);
    if (bytes_received == -1)
    {
        perror("Error receiving response from storage server 1");
        logMessage(logFilePath, "Error receiving response from storage server 1");
        close(storage_server1);
        close(storage_server2);
    }
    if (source_response.source_ss_confirmation == 1)
    {
        sprintf(tempLog, "Source storage server %d confirmed the operation.\nChecking For storage server %d\n", copyInfo.ss_source_id, copyInfo.ss_destination_id);
        logMessage(logFilePath, tempLog);
        printf(tempLog);
    }
    else
    {
        sprintf(tempLog, "Source storage server %d denied the operation.\n", copyInfo.ss_source_id);
        printf(tempLog);
        close(storage_server1);
        close(storage_server2);
        return;
    }

    bytes_sent = send(storage_server2, &request, sizeof(struct ClientRequest), 0);
    if (bytes_sent == -1)
    {
        perror("Error sending request to storage server 2");
        close(storage_server1);
        close(storage_server2);
        exit(EXIT_FAILURE);
    }
    bytes_received = recv(storage_server2, &destination_response, sizeof(struct NM_to_SS_Response), 0);
    if (bytes_received == -1)
    {
        perror("Error receiving response from storage server 2");
        close(storage_server1);
        close(storage_server2);
        exit(EXIT_FAILURE);
    }
    if (destination_response.destination_ss_confirmation == 1)
    {
        printf("Destination storage server %d confirmed the operation.\n", copyInfo.ss_destination_id);
    }
    else
    {
        printf("Destination storage server %d denied the operation.\n", copyInfo.ss_destination_id);
        close(storage_server1);
        close(storage_server2);
        exit(EXIT_FAILURE);
    }

    // starting the operation
    source_response.start_operation = 1;
    bytes_sent = send(storage_server1, &source_response, sizeof(struct NM_to_SS_Response), 0);
    if (bytes_sent == -1)
    {
        perror("Error sending start operation to storage server 1");
        close(storage_server1);
        close(storage_server2);
        close(main_server);
        return;
    }
    while (1)
    {
        struct Packet packet;
        bytes_received = recv(storage_server1, &packet, sizeof(struct Packet), 0);
        if (bytes_received == -1)
        {
            perror("Error receiving data from storage server 1");
            close(storage_server1);
            close(storage_server2);
            close(main_server);
            return;
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by storage server 1\n");
            close(storage_server1);
            close(storage_server2);
            close(main_server);
            return;
        }
        else
            printf("Received: %s\n", packet.data);
        {

            bytes_sent = send(storage_server2, &packet, sizeof(struct Packet), 0);
            if (bytes_sent == -1)
            {
                perror("Error sending data to storage server 2");
                close(storage_server1);
                close(storage_server2);
                close(main_server);
                return;
            }

            bytes_sent = send(storage_server1, &packet, sizeof(struct Packet), 0);
            if (bytes_sent == -1)
            {
                perror("Error sending data to storage server 1");
                close(storage_server1);
                close(storage_server2);
                close(main_server);
                return;
            }
            if (packet.isEnd == 1)
            {
                break;
            }
        }
    }

    printf("NM : File copied successfully\n");
    logMessage(logFilePath, "File copied successfully\n");
    // Close sockets
    close(storage_server1);
    close(storage_server2);

    return;
}

void add_file_directory(char *filename, int type, int ssid)
{
    if (type == 1)
    {
        // realloc combinedFileinfo[ssid].files and add the above filename
        if (combinedFilesInfoAll[ssid].files == NULL)
        {
            printf("I am null\n\n");
            combinedFilesInfoAll[ssid].files = (struct FileInfo *)malloc(sizeof(struct FileInfo));
        }
        else
        {
            printf("I am not null\n\n");
            combinedFilesInfoAll[ssid].files = (struct FileInfo *)realloc(combinedFilesInfoAll[ssid].files, sizeof(struct FileInfo) * (combinedFilesInfoAll[ssid].numberOfFiles + 1));
        }
        strcpy(combinedFilesInfoAll[ssid].files[combinedFilesInfoAll[ssid].numberOfFiles].name, filename);
        printf("Added file %s to storage server %d\n", combinedFilesInfoAll[ssid].files[combinedFilesInfoAll[ssid].numberOfFiles].name, ssid);
        combinedFilesInfoAll[ssid].numberOfFiles++;
        storageServers[ssid].numberOfFiles++;

        add_file_in_hash(filename, ssid, fileshash);
    }
    else if (type == 2)
    {
        // realloc combinedFileinfo[ssid].directories and add the above filename
        if (combinedFilesInfoAll[ssid].directories == NULL)
        {
            combinedFilesInfoAll[ssid].directories = (struct DirectoryInfo *)malloc(sizeof(struct DirectoryInfo));
        }
        else
        {
            combinedFilesInfoAll[ssid].directories = (struct DirectoryInfo *)realloc(combinedFilesInfoAll[ssid].directories, sizeof(struct DirectoryInfo) * (combinedFilesInfoAll[ssid].numberOfDirectories + 1));
        }
        strcpy(combinedFilesInfoAll[ssid].directories[combinedFilesInfoAll[ssid].numberOfDirectories].name, filename);
        combinedFilesInfoAll[ssid].numberOfDirectories++;
        storageServers[ssid].numberOfDirectories++;

        add_dir_in_hash(filename, ssid, dirhash);
    }
    else
    {
        printf("[-]Invalid type\n");
    }
}

void remove_file_directory(char *filename, int type, int ssid)
{
    if (type == 1)
    {
        // realloc combinedFileinfo[ssid].files and add the above filename
        ff temp = fileSearchWithHash(filename, fileshash);
        printf("File found on storage server %d\n", temp.ssid);
        printf("file path : %s\n", temp.filepath.name);

        int indexToRemove = -1;
        for (int i = 0; i < storageServers[ssid].numberOfFiles; i++)
        {
            if (strcmp(combinedFilesInfoAll[ssid].files[i].name, temp.filepath.name) == 0)
            {
                indexToRemove = i;
                break;
            }
        }

        if (indexToRemove == -1)
        {
            printf("File not found in the array.\n");
            return;
        }
        else
        {
            printf("File found in the array index %d.\n", indexToRemove);
            printf("File name : %s\n", combinedFilesInfoAll[ssid].files[indexToRemove].name);
        }

        // Shift elements to the left to fill the gap
        if (combinedFilesInfoAll[ssid].numberOfFiles == 1)
        {
            free(combinedFilesInfoAll[ssid].files);
            combinedFilesInfoAll[ssid].files = NULL;
        }
        else
        {

            for (int i = indexToRemove; i < storageServers[ssid].numberOfFiles - 1; i++)
            {
                printf("BEFORE: File name : %s\n", combinedFilesInfoAll[ssid].files[i].name);
                strcpy(combinedFilesInfoAll[ssid].files[i].name, combinedFilesInfoAll[ssid].files[i + 1].name);
                printf("AFTER: File name : %s\n", combinedFilesInfoAll[ssid].files[i].name);
            }
        }

        combinedFilesInfoAll[ssid].numberOfFiles--;
        storageServers[ssid].numberOfFiles--;
        if (combinedFilesInfoAll[ssid].numberOfFiles == 0)
        {
            combinedFilesInfoAll[ssid].files = (struct FileInfo *)realloc(combinedFilesInfoAll[ssid].files, sizeof(struct FileInfo) * (combinedFilesInfoAll[ssid].numberOfFiles));
        }

        removeHashEntryFile(filename, fileshash);
        // printf("BEFORE TREE\n");
        // printTree(&dirTree, 0);
        removeFromTree(&dirTree, 0, filename);
        // printf("AFTER TREE\n");
        // printTree(&dirTree, 0);
    }
    else if (type == 2)
    {

        dd temp = dirSearchWithHash(filename, dirhash);
        printf("Directory found on storage server %d\n", temp.ssid);
        printf("Directory path : %s\n", temp.dirpath.name);

        int indexToRemove = -1;
        for (int i = 0; i < storageServers[ssid].numberOfDirectories; i++)
        {
            if (strcmp(combinedFilesInfoAll[ssid].directories[i].name, temp.dirpath.name) == 0)
            {
                indexToRemove = i;
                break;
            }
        }

        if (indexToRemove == -1)
        {
            printf("Directory not found in the array.\n");
            return;
        }
        else
        {
            printf("Directory found in the array index %d.\n", indexToRemove);
            printf("Directory name : %s\n", combinedFilesInfoAll[ssid].directories[indexToRemove].name);
        }

        // Shift elements to the left to fill the gap
        if (storageServers[ssid].numberOfDirectories == 1)
        {
            free(combinedFilesInfoAll[ssid].directories);
            combinedFilesInfoAll[ssid].directories = NULL;
        }
        else
        {
            for (int i = indexToRemove; i < storageServers[ssid].numberOfDirectories - 1; i++)
            {
                printf("BEFORE: Directory name : %s\n", combinedFilesInfoAll[ssid].directories[i].name);
                strcpy(combinedFilesInfoAll[ssid].directories[i].name, combinedFilesInfoAll[ssid].directories[i + 1].name);
                printf("AFTER: Directory name : %s\n", combinedFilesInfoAll[ssid].directories[i].name);
            }
        }

        // realloc combinedFileinfo[ssid].directories and add the above filename
        combinedFilesInfoAll[ssid].numberOfDirectories--;
        storageServers[ssid].numberOfDirectories--;
        if (combinedFilesInfoAll[ssid].numberOfDirectories != 0)
        {
            combinedFilesInfoAll[ssid].directories = (struct DirectoryInfo *)realloc(combinedFilesInfoAll[ssid].directories, sizeof(struct DirectoryInfo) * (combinedFilesInfoAll[ssid].numberOfDirectories));
        }
        removeHashEntryDirectory(filename, dirhash);
        // printf("BEFORE TREE\n");
        // printTree(&dirTree, 0);
        removeFromTree(&dirTree, 1, filename);
        // printf("AFTER TREE\n");
        // printTree(&dirTree, 0);
    }
    else
    {
        printf("[-]Invalid type\n");
    }
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

void *ss_connection(void *arg)
{
    struct ClientRequest *request = (struct ClientRequest *)arg;
    struct ClientRequest temp;
    //* copy request to temp
    temp.clientID = request->clientID;
    temp.transactionId = request->transactionId;
    strcpy(temp.command, request->command);
    strcpy(temp.arguments[0], request->arguments[0]);
    strcpy(temp.arguments[1], request->arguments[1]);

    int ss_id = 0;

    if (strcmp(request->command, "DELETE") == 0)
    {
        //* find the file from hash table
        if (strcmp(request->arguments[0], "FILE") == 0)
        {
            ff file;
            file.ssid = -1;
            file = fileSearchWithHash(request->arguments[1], fileshash);
            printf("File found on storage server %d\n", file.ssid);
            printf("file path : %s\n", file.filepath.name);
            ss_id = file.ssid;
            if (ss_id == -1)
            {
                printf("File not found in the hash table\n");
                pthread_exit(NULL);
            }
        }
        else if (strcmp(request->arguments[0], "DIR") == 0)
        {
            dd dir;
            dir.ssid = -1;
            dir = dirSearchWithHash(request->arguments[1], dirhash);
            printf("Directory found on storage server %d\n", dir.ssid);
            printf("Directory path : %s\n", dir.dirpath.name);
            ss_id = dir.ssid;
            if (ss_id == -1)
            {
                printf("Directory not found in the hash table\n");
                pthread_exit(NULL);
            }
        }
    }
    else if (strcmp(request->command, "COPY") == 0)
    {
        copy_file_between_ss(temp);
        pthread_exit(NULL);
    }
    else if (strcmp(request->command, "CREATE") == 0)
    {
        struct TreeClosestDirPacket tcdp;
        tcdp = closestDir(&dirTree, request->arguments[1]);
        printf("Closest dir : %s (Storage server %d)\n", tcdp.dirinfo.name, tcdp.ssid);
        ss_id = tcdp.ssid;
        ;
        //* assign appropriate ssid here
    }

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
    addr.sin_port = htons(storageServers[ss_id].storageServerPort);
    addr.sin_addr.s_addr = inet_addr(storageServers[ss_id].ipAddress);

    size_address = sizeof(addr);

    int connection_tries = 0;
    while (1)
    {
        if (connect(socket_ss, (struct sockaddr *)&addr, size_address) == 0)
        {
            printf("[+]Connected to Storage-Server %d to perform operation %s.\n", ss_id + 1, request->command);
            break;
        }
        else
        {
            printf("[-]Connection attempt %d/%d to Storage-Server %d on PORT %d failed. Trying again...\n", connection_tries + 1, MAX_CONNECTION_TRIES, ss_id + 1, storageServers[ss_id].storageServerPort);
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

    int bytes_sent, bytes_received;
    bytes_sent = send(socket_ss, &temp, sizeof(temp), 0);

    printf("Request sent to storage server. Waiting for response...\n");

    struct NM_to_SS_Response response;
    if (bytes_sent == -1)
    {
        perror("Error sending heartbeat to storage server");
        close(socket_ss);
        printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
    }
    bytes_received = recv(socket_ss, &response, sizeof(response), 0);
    printf("Response received from storage server.\n");
    if (bytes_received == -1)
    {
        perror("Error receiving heartbeat from storage server");
        close(socket_ss);
        printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
    }
    else if (bytes_received == 0)
    {
        printf("Connection closed by the Storage-Server.\n");
        close(socket_ss);
        printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
    }
    request->operation_status = response.operation_status;
    if (response.operation_status == 1)
    {
        printf("Operation performed successfully\n");
        printf("Updating hash table\n");
        //* if create was done, call add to hash table
        if (strcmp(request->command, "CREATE") == 0)
        {
            if (strcmp(request->arguments[0], "FILE") == 0)
            {
                printf("\n\n====\nI am going into file iffff\n\n====\n");
                printf("Adding file %s to hash table\n", request->arguments[1]);
                add_file_directory(request->arguments[1], 1, ss_id);
                for (int i = 0; i < response.new_dir_count; i++)
                {
                    add_file_directory(response.dir[i].name, 2, ss_id);
                }
            }
            else if (strcmp(request->arguments[0], "DIR") == 0)
            {
                for (int i = 0; i < response.new_dir_count; i++)
                {
                    add_file_directory(response.dir[i].name, 2, ss_id);
                }
            }
        }
        else if (strcmp(request->command, "DELETE") == 0)
        {
            if (strcmp(request->arguments[0], "FILE") == 0)
            {
                remove_file_directory(request->arguments[1], 1, ss_id);
            }
            else if (strcmp(request->arguments[0], "DIR") == 0)
            {
                for (int i = 0; i < response.new_dir_count; i++)
                {
                    remove_file_directory(response.dir[i].name, 2, ss_id);
                }
                for (int i = 0; i < response.new_file_count; i++)
                {
                    remove_file_directory(response.file[i].name, 1, ss_id);
                }
            }
            // print_hash_table_directories(dirhash);
            // print_hash_table_files(fileshash);
            printf("\n\nPrinting storage server info after deletion\n");
            print_ss_info(&storageServers[ss_id], combinedFilesInfoAll[ss_id]);
        }

        // print_hash_table_files(fileshash);
        // print_hash_table_directories(dirhash);
        // else if(strcmp(request->command, "COPY") == 0)
        // {
        //     if (strcmp(request->arguments[0], "FILE") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 1, ss_id);
        //     }
        //     else if (strcmp(request->arguments[0], "DIR") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 2, ss_id);
        //     }
        // }
        // else if(strcmp(request->command, "WRITE") == 0)
        // {
        //     if (strcmp(request->arguments[0], "FILE") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 1, ss_id);
        //     }
        //     else if (strcmp(request->arguments[0], "DIR") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 2, ss_id);
        //     }
        // }
        // else if(strcmp(request->command, "READ") == 0)
        // {
        //     if (strcmp(request->arguments[0], "FILE") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 1, ss_id);
        //     }
        //     else if (strcmp(request->arguments[0], "DIR") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 2, ss_id);
        //     }
        // }
        // else if(strcmp(request->command, "FILEINFO") == 0)
        // {
        //     if (strcmp(request->arguments[0], "FILE") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 1, ss_id);
        //     }
        //     else if (strcmp(request->arguments[0], "DIR") == 0)
        //     {
        //         add_file_directory(request->arguments[1], 2, ss_id);
        //     }
        // }
        print_ss_info(&storageServers[ss_id], combinedFilesInfoAll[ss_id]);
    }
    else
    {
        printf("Operation failed\n");
    }
    pthread_exit(NULL);
}

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

                if (strcmp(client_request.command, "LIST") == 0)
                {
                    char filename[MAX_PATH_SIZE] = "merged_paths.txt";
                    createFileFromArray(filename, combinedFilesInfoAll, storageServers, number_of_ss);
                    response.operation_performer = 1;

                    printf("Sending response to client...\n");
                    bytes_sent = send(socket_client, &response, sizeof(response), 0);
                    if (bytes_sent == -1)
                    {
                        perror("Error sending data to Client");
                        close(socket_client);
                        close(socket_server_nm);
                        pthread_exit(NULL);
                    }

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
                        if (client_request.start_operation == 1)
                        {
                            printf("Client ready to receive the file...\n");
                            sendFile(socket_client, filename);
                            printf("File sent to client\n");
                        }
                    }
                }
                else if (strcmp(client_request.command, "READ") == 0 || strcmp(client_request.command, "FILEINFO") == 0 || strcmp(client_request.command, "WRITE") == 0)
                {
                    ff file;
                    file.ssid = -1;
                    printf("Searching for file...\n");
                    file = fileSearchWithHash(client_request.arguments[0], fileshash);
                    printf("File found on storage server %d\n", file.ssid);
                    printf("file path : %s\n", file.filepath.name);

                    response.operation_performer = 2;
                    response.ss_port = storageServers[file.ssid].clientPort;
                    strcpy(response.ss_ip, storageServers[file.ssid].ipAddress);
                    strcpy(response.file.name, file.filepath.name);

                    print_response_info(response);
                    response.operation_status = 0;

                    bytes_sent = send(socket_client, &response, sizeof(response), 0);
                    if (bytes_sent == -1)
                    {
                        perror("Error sending data to Client");
                        close(socket_client);
                        close(socket_server_nm);
                        pthread_exit(NULL);
                    }
                }
                else if (strcmp(client_request.command, "CREATE") == 0 || strcmp(client_request.command, "DELETE") == 0 || strcmp(client_request.command, "COPY") == 0)
                {
                    ff file;
                    file.ssid = -1;
                    file = fileSearchWithHash(client_request.arguments[1], fileshash);
                    printf("File found on storage server %d\n", file.ssid);
                    printf("file path : %s\n", file.filepath.name);

                    strcpy(response.file.name, file.filepath.name);
                    response.operation_performer = 1;

                    print_response_info(response);

                    pthread_t ss_operation;

                    printf("Creating thread for storage server operation\n");

                    pthread_create(&ss_operation, NULL, ss_connection, (void *)&client_request);
                    pthread_join(ss_operation, NULL);

                    printf("Thread for storage server operation joined\n");
                    response.operation_status = 1;

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
            int server_exists = compareFilePath(ss.ss_boot_path, storageServers, number_of_ss);
            if (server_exists == -1)
            {
                ss.storageServerID = number_of_ss;
                combinedFilesInfo.storageServerID = ss.storageServerID;
                assign_ports_ss(&ss);

                combinedFilesInfoAll[number_of_ss] = combinedFilesInfo;
                storageServers[number_of_ss] = ss;

                store_in_hash_file(&combinedFilesInfoAll[number_of_ss], fileshash);
                store_in_hash_dir(&combinedFilesInfoAll[number_of_ss], dirhash);
                //* send updated storage-server
                bytes_sent = send(socket_ss, &ss, sizeof(ss), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending SS-ID to storage server");
                    close(socket_ss);
                    break;
                }

                print_ss_info(&ss, combinedFilesInfoAll[ss.storageServerID]);
                struct storage_server_connection_struct ss_struct;
                ss_struct.ss_id = number_of_ss;

                fillDirTree(&dirTree, combinedFilesInfoAll);
                //* write a function to print the tree
                printTree(&dirTree, 0);

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

                print_ss_info(&storageServers[server_exists], combinedFilesInfoAll[ss.storageServerID]);
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
    initialize_hash_table(fileshash, dirhash);
    initialiseDirTree(&dirTree);

    //* thread for listening to first connection from storage server
    pthread_create(&first_connection_ss_thread, NULL, first_connection_ss, NULL);
    pthread_create(&first_connection_client_thread, NULL, first_connection_client, NULL);

    pthread_join(first_connection_ss_thread, NULL);
    pthread_join(first_connection_client_thread, NULL);
    return 0;
}
