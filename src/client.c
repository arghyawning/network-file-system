#include "../include/common.h"

struct ClientInfo client;

struct ClientRequest clientRequest;

void flushInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void readAndDisplayData(int socket)
{
    char *buffer = NULL;
    size_t size = 0;
    ssize_t bytesRead;

    printf("FILE Begin ==============================================================\n\n");

    while (1)
    {
        struct Packet packet;
        bytesRead = recv(socket, &packet, sizeof(struct Packet), 0);

        if (bytesRead == -1)
        {
            perror("[-] Error in receiving data");
            exit(EXIT_FAILURE);
        }
        else if (bytesRead == 0)
        {
            // No more data to receive
            break;
        }

        if (packet.isEnd)
        {
            // End of file
            break;
        }

        // Dynamically allocate memory for the buffer
        // printf("Received packet number %d\n", packet.packetNumber);
        // size += strlen(packet.data);
        // buffer = realloc(buffer, size);
        // if (buffer == NULL)
        // {
        //     perror("[-] Memory allocation failed");
        //     exit(EXIT_FAILURE);
        // }

        // // Append received data to the buffer
        // strcat(buffer, packet.data);
        printf("%s", packet.data);
    }

    printf("\n\nFILE End ==============================================================\n\n");

    // Display the received data
    // printf("Received data:\n%s\n", buffer);

    // Free the allocated memory
    // free(buffer);
}

void displayAccessiblePaths(int socket)
{
    char *buffer = NULL;
    size_t size = 0;
    ssize_t bytesRead;

    printf("=============================== Accessible Paths ==============================\n\n");

    while (1)
    {
        struct Packet packet;
        bytesRead = recv(socket, &packet, sizeof(struct Packet), 0);

        if (bytesRead == -1)
        {
            perror("[-] Error in receiving data");
            exit(EXIT_FAILURE);
        }
        else if (bytesRead == 0)
        {
            // No more data to receive
            break;
        }

        if (packet.isEnd)
        {
            // End of file
            break;
        }

        // Dynamically allocate memory for the buffer
        // printf("Received packet number %d\n", packet.packetNumber);
        // size += strlen(packet.data);
        // buffer = realloc(buffer, size);
        // if (buffer == NULL)
        // {
        //     perror("[-] Memory allocation failed");
        //     exit(EXIT_FAILURE);
        // }

        // // Append received data to the buffer
        // strcat(buffer, packet.data);
        printf("%s", packet.data);
    }

    printf("\n\n===============================================================================\n\n");

    // Display the received data
    // printf("Received data:\n%s\n", buffer);

    // Free the allocated memory
    // free(buffer);
}

void DisplayFileInfo(int socket, char *filepath)
{
    struct stat fileStat;

    struct FileInfo file;
    int bytes_received = recv(socket, &fileStat, sizeof(fileStat), 0);
    if (bytes_received == -1)
    {
        perror("Error receiving heartbeat from storage server");
        close(socket);
        // storageServers[ss_id].isConnected = 0;
        // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
        return;
    }
    else if (bytes_received == 0)
    {
        printf("Connection closed by the Storage-Server.\n");
        close(socket);
        // storageServers[ss_id].isConnected = 0;
        // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
        return;
    }

    char *filename = basename((char *)filepath);
    char *file_extension = strrchr(filename, '.');

    printf("Information of for %d\n", filename);
    printf("---------------------------\n");
    printf("File Path: \t\t%s\n", filepath);
    printf("File Extension: \t%s\n", (file_extension != NULL) ? (file_extension + 1) : "N/A");
    printf("File Size: \t\t%ld bytes\n", fileStat.st_size);
    printf("File Mode: \t\t%d\n", fileStat.st_mode);
    printf("Number of Links: \t%ld\n", fileStat.st_nlink);
    printf("File inode: \t\t%ld\n", fileStat.st_ino);
    printf("File UID: \t\t%d\n", fileStat.st_uid);
    printf("File GID: \t\t%d\n", fileStat.st_gid);
    printf("File Blocks: \t\t%ld\n", fileStat.st_blocks);
    printf("Last Accessed: \t\t%s", ctime(&fileStat.st_atime));
    printf("Last Modified: \t\t%s", ctime(&fileStat.st_mtime));
    printf("Last Status Change: \t%s", ctime(&fileStat.st_ctime));

    // Print file permissions
    printf("File Permissions: ");

    // Owner permissions
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");

    // Group permissions
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");

    // Others permissions
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf("\n");
}

void send_data_in_chunks(const char *data, int data_length, int sockfd)
{
    int bytes_sent = 0;
    int packetNumber = 1; // Starting packet number

    while (bytes_sent < data_length)
    {
        struct PacketWrite packet;
        memset(&packet, 0, sizeof(struct PacketWrite));

        // Set packet attributes
        packet.isStart = (bytes_sent == 0) ? 1 : 0;
        packet.isEnd = (bytes_sent + MAX_CHUNK_SIZE >= data_length) ? 1 : 0;
        packet.packetNumber = packetNumber;

        // Determine the size to copy based on remaining data
        int size_to_copy = (packet.isEnd) ? (data_length - bytes_sent) : MAX_CHUNK_SIZE;
        packet.datalength = size_to_copy;

        // Copy the data into the packet
        strncpy(packet.data, data + bytes_sent, size_to_copy);
        //! DO NOT DO THIS
        //! packet.data[packet.datalength] = '\0';
        packet.data[size_to_copy] = '\0'; // Null-terminate the data
        // strncpy(packet.data, data + bytes_sent, MAX_CHUNK_SIZE);

        // Send the packet to the server
        if (send(sockfd, &packet, sizeof(struct PacketWrite), 0) == -1)
        {
            perror("Error sending data to server");
            exit(EXIT_FAILURE);
        }
        printf("data sent: %s\n", packet.data);

        bytes_sent += MAX_CHUNK_SIZE;
        packetNumber++;
    }
}

void writeFile(int socket, char *filepath)
{
    flushInputBuffer();
    printf(">>Enter the data to be written into the file\n");

    char *buffer = NULL; // Initialize buffer pointer
    size_t size = 0;     // Initialize size of the buffer
    size_t index = 0;    // Initialize index for tracking the position in the buffer

    int ch;
    while ((ch = fgetc(stdin)) != '\n' && ch != EOF)
    {
        if (index == size)
        {
            size = (size == 0) ? 1 : size * 2;
            buffer = realloc(buffer, size * sizeof(char));

            if (buffer == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
        }

        buffer[index++] = (char)ch;
    }

    // Null-terminate the string
    buffer = realloc(buffer, (index + 1) * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    buffer[index] = '\0';

    printf("content to write: %s", buffer);
    // printf("Input received: %s\n", buffer);

    send_data_in_chunks(buffer, index, socket);
    // size_t data_length = strlen(buffer);
    // size_t bytes_sent = 0;

    // while (bytes_sent < data_length)
    // {
    //     size_t chunk_size = (data_length - bytes_sent) < MAX_CHUNK_SIZE ? (data_length - bytes_sent) : MAX_CHUNK_SIZE;

    //     if (send(socket, buffer + bytes_sent, chunk_size, 0) == -1)
    //     {
    //         perror("Error sending data to server");
    //         exit(EXIT_FAILURE);
    //     }

    //     bytes_sent += chunk_size;
    // }
}

uint64_t generateTransactionId(uint32_t clientId)
{
    //* Using a combination of timestamp, client ID, and randomness
    srandom(time(NULL));
    uint64_t timestamp = (uint64_t)time(NULL);
    uint64_t randomness = random();
    uint64_t transactionId = (timestamp << 32) | (clientId << 16) | randomness;

    return transactionId;
}

// check if path is invalid
int invalidPath(char *path)
{
    if (access(path, F_OK) == -1)
    {
        printf("Invalid path: %s\n", path);
        return 1;
    }
    return 0;
}

// error display
void errorDisplay(int flag)
{
    switch (flag)
    {
    case 1:
        printf("No input entered\n");
        break;
    case 2:
        printf("Invalid command: Too less arguments\n");
        break;
    case 3:
        printf("Invalid command\n");
        break;
    default:
        printf("Error!\n");
        break;
    }
}

void add_prefix_if_missing(char *input_string)
{
    if (strncmp(input_string, "./", 2) != 0)
    {
        memmove(input_string + 2, input_string, strlen(input_string) + 1);
        strncpy(input_string, "./", 2);
    }
}

void inputParse(char *input)
{
    char *command;

    command = strtok(input, " ");
    if (command == NULL)
    {
        errorDisplay(1);
        return;
    }

    char *filepath = strtok(NULL, " ");
    if (strcmp(command, "LIST") == 0)
    {
        strcpy(clientRequest.command, "LIST");
        strcpy(clientRequest.arguments[0], "");
        strcpy(clientRequest.arguments[1], "");
    }
    else if (strcmp(command, "READ") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        // if (invalidPath(filepath))
        //     return;

        // custom_read(filepath);
        strcpy(clientRequest.command, "READ");
        strcpy(clientRequest.arguments[0], filepath);
        strcpy(clientRequest.arguments[1], "");
        if (strlen(clientRequest.arguments[0]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[0]);
            printf("Modified path: %s\n", clientRequest.arguments[0]);
        }
    }
    else if (strcmp(command, "WRITE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        // if (invalidPath(filepath))
        //     return;

        // custom_write(filepath);
        strcpy(clientRequest.command, "WRITE");
        strcpy(clientRequest.arguments[0], filepath);
        strcpy(clientRequest.arguments[1], "");

        if (strlen(clientRequest.arguments[0]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[0]);
            printf("Modified path: %s\n", clientRequest.arguments[0]);
        }
    }
    else if (strcmp(command, "DELETE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        char *type, *filename;
        type = (char *)malloc(sizeof(char) * 16);
        strcpy(type, filepath);

        filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            errorDisplay(2);
            return;
        }

        // custom_create(filepath);
        strcpy(clientRequest.command, "DELETE");
        strcpy(clientRequest.arguments[0], type);
        strcpy(clientRequest.arguments[1], filename);

        if (strlen(clientRequest.arguments[1]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[1]);
            printf("Modified path: %s\n", clientRequest.arguments[1]);
        }
    }
    else if (strcmp(command, "COPY") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }
        // if (invalidPath(filepath))
        //     return;
        printf("srcpath: \n");

        char *srcpath, *destpath;
        srcpath = (char *)malloc(sizeof(char) * MAX_PATH_SIZE);
        strcpy(srcpath, filepath);

        destpath = strtok(NULL, " ");
        if (destpath == NULL)
        {
            errorDisplay(2);
            return;
        }
        // if (invalidPath(destpath))
        //     return;

        // custom_copy(srcpath, destpath);
        strcpy(clientRequest.command, "COPY");
        strcpy(clientRequest.arguments[0], srcpath);
        strcpy(clientRequest.arguments[1], destpath);

        if (strlen(clientRequest.arguments[0]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[0]);
            printf("Modified path: %s\n", clientRequest.arguments[0]);
        }

        if (strlen(clientRequest.arguments[1]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[1]);
            printf("Modified path: %s\n", clientRequest.arguments[1]);
        }
    }
    else if (strcmp(command, "CREATE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        char *type, *filename;
        type = (char *)malloc(sizeof(char) * 16);
        strcpy(type, filepath);

        filename = strtok(NULL, " ");
        if (filename == NULL)
        {
            errorDisplay(2);
            return;
        }

        // custom_create(filepath);
        strcpy(clientRequest.command, "CREATE");
        strcpy(clientRequest.arguments[0], type);
        strcpy(clientRequest.arguments[1], filename);

        if (strlen(clientRequest.arguments[1]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[1]);
            printf("Modified path: %s\n", clientRequest.arguments[1]);
        }
    }
    else if (strcmp(command, "FILEINFO") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        // if (invalidPath(filepath))
        //     return;

        // custom_fileinfo(filepath);
        strcpy(clientRequest.command, "FILEINFO");
        strcpy(clientRequest.arguments[0], filepath);
        strcpy(clientRequest.arguments[1], "");

        if (strlen(clientRequest.arguments[0]) > 0)
        {
            add_prefix_if_missing(clientRequest.arguments[0]);
            printf("Modified path: %s\n", clientRequest.arguments[0]);
        }
    }
    else
    {
        errorDisplay(3);
    }

    return;
}

void read_parse_input()
{
    char *buffer = NULL; // Initialize buffer pointer
    size_t size = 0;     // Initialize size of the buffer
    size_t index = 0;    // Initialize index for tracking the position in the buffer

    printf("\n>> Enter Input:\n");

    int ch;
    while ((ch = fgetc(stdin)) != '\n' && ch != EOF)
    {
        if (index == size)
        {
            size = (size == 0) ? 1 : size * 2;
            buffer = realloc(buffer, size * sizeof(char));

            if (buffer == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
        }

        buffer[index++] = (char)ch;
    }

    // Null-terminate the string
    buffer = realloc(buffer, (index + 1) * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    buffer[index] = '\0';

    printf("Input received: %s\n", buffer);

    // ssize_t bytesRead = getline(&buffer, &size, stdin);

    // if (bytesRead == -1)
    // {
    //     fprintf(stderr, "Error reading input\n");
    //     exit(EXIT_FAILURE);
    // }

    // // Remove newline character, if present
    // if (bytesRead > 0 && buffer[bytesRead - 1] == '\n')
    //     buffer[bytesRead - 1] = '\0';

    // printf("Input received: %s\n", buffer);

    // Free the allocated memory

    inputParse(buffer);
    clientRequest.transactionId = generateTransactionId(client.clientID);
    clientRequest.clientID = client.clientID;

    free(buffer);

    return;
}

void *perform_operation_NM(void *arg)
{
    printf("Performing operation on Naming-Server\n");
    return NULL;
}

void *perform_operation_ss(void *arg)
{
    struct Client_to_NM_response response = *(struct Client_to_NM_response *)arg;
    int socket_client;
    struct sockaddr_in addr;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_client < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(response.ss_port);
    addr.sin_addr.s_addr = inet_addr(response.ss_ip);

    int connection_tries = 0;
    while (1)
    {
        if (connect(socket_client, (struct sockaddr *)&addr, sizeof(addr)) == 0)
        {
            printf("[+]Connected to Storage-Server on Port %d.\n", client.clientPort);
            break;
        }
        else
        {
            printf("[-]Connection attempt %d/%d to Storage-Server on PORT %d failed. Trying again...\n", connection_tries + 1, MAX_CONNECTION_TRIES, client.clientPort);
            connection_tries++;
            sleep(RETRY_INTERVAL);
        }

        if (connection_tries == MAX_CONNECTION_TRIES)
        {
            printf("[-]Could not connect to Naming-Server. Stopping further connection attempts\n");
            close(socket_client);
            pthread_exit(NULL);
        }
    }

    int bytes_sent, bytes_received;
    struct Client_to_SS_Request request_ss;
    strcpy(request_ss.command, clientRequest.command);
    strcpy(request_ss.file.name, response.file.name);
    strcpy(request_ss.buffer, "Hello from client");

    if (strcmp(clientRequest.command, "WRITE") == 0)
    {
        printf("Choose a write method\n1. Overwrite\n2. Append\n");
        int choice;
        scanf("%d", &choice);
        request_ss.write_type = choice;
    }

    print_client_request_info(clientRequest);

    bytes_sent = send(socket_client, &request_ss, sizeof(request_ss), 0);

    if (bytes_sent == -1)
    {
        perror("Error sending heartbeat to storage server");
        close(socket_client);
        pthread_exit(NULL);
    }

    //* READ
    if (strcmp(clientRequest.command, "READ") == 0)
    {
        printf("Reading data from storage server\n");
        readAndDisplayData(socket_client);
        printf("Data read successfully\n");
    }
    else if (strcmp(clientRequest.command, "FILEINFO") == 0)
    {
        printf("Reading data from storage server\n");
        DisplayFileInfo(socket_client, clientRequest.arguments[0]);
        printf("Data read successfully\n");
    }
    else if (strcmp(clientRequest.command, "WRITE") == 0)
    {
        printf("Writing data to storage server\n");
        writeFile(socket_client, request_ss.file.name);
        printf("Data written successfully\n");
    }

    printf("Exiting Thread\n");
    pthread_exit(NULL);
    // bytes_received = recv(socket_client, &request_ss, sizeof(request_ss), 0);

    // if (bytes_received == -1)
    // {
    //     perror("Error receiving heartbeat from storage server");
    //     close(socket_client);
    //     break;
    // }
    // else if (bytes_received == 0)
    // {
    //     printf("Connection closed by the Storage-Server.\n");
    //     close(socket_client);
    //     break;
    // }
}

void *naming_server_connection(void *arg)
{
    int socket_client;
    struct sockaddr_in addr;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_client < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(client.clientPort);
    addr.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);

    int connection_tries = 0;
    while (1)
    {
        if (connect(socket_client, (struct sockaddr *)&addr, sizeof(addr)) == 0)
        {
            printf("[+]Connected to Naming-Server on Port %d.\n", client.clientPort);
            break;
        }
        else
        {
            printf("[-]Connection attempt %d/%d to Naming-Server on PORT %d failed. Trying again...\n", connection_tries + 1, MAX_CONNECTION_TRIES, client.clientPort);
            connection_tries++;
            sleep(RETRY_INTERVAL);
        }

        if (connection_tries == MAX_CONNECTION_TRIES)
        {
            printf("[-]Could not connect to Naming-Server. Stopping further connection attempts\n");
            close(socket_client);
            pthread_exit(NULL);
        }
    }

    int ticks = 0;
    while (1)
    {
        int bytes_sent, bytes_received;
        struct Client_to_NM_response response;
        pthread_t operation_thread;
        //* Take input from user
        read_parse_input();
        print_client_request_info(clientRequest);
        bytes_sent = send(socket_client, &clientRequest, sizeof(clientRequest), 0);

        if (bytes_sent == -1)
        {
            perror("Error sending heartbeat to storage server");
            close(socket_client);
            // storageServers[ss_id].isConnected = 0;
            // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        bytes_received = recv(socket_client, &response, sizeof(response), 0);
        if (bytes_received == -1)
        {
            perror("Error receiving heartbeat from storage server");
            close(socket_client);
            // storageServers[ss_id].isConnected = 0;
            // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed by the Storage-Server.\n");
            close(socket_client);
            // storageServers[ss_id].isConnected = 0;
            // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
            break;
        }
        if (response.transactionId != clientRequest.transactionId)
        {
            printf("Transaction ID mismatch. Expected: %lu, Received: %lu\n", clientRequest.transactionId, response.transactionId);
            continue;
        }
        print_response_info(response);

        if (response.operation_performer == 1)
        {

            print_client_request_info(clientRequest);
            if (strcmp(clientRequest.command, "LIST") == 0)
            {

                printf("Starting to list...\n");
                clientRequest.start_operation = 1;
                bytes_sent = send(socket_client, &clientRequest, sizeof(clientRequest), 0);
                if (bytes_sent == -1)
                {
                    perror("Error sending heartbeat to storage server");
                    close(socket_client);
                    // storageServers[ss_id].isConnected = 0;
                    // printf("[-]Connection to Storage-Server %d lost.\n", ss_id + 1);
                    break;
                }
                displayAccessiblePaths(socket_client);
            }
            
            if (clientRequest.operation_status == 1)
            {
                if (strcmp(clientRequest.command, "CREATE"))
                {
                    if (strcmp(clientRequest.arguments[0], "FILE"))
                    {
                        printf("File created successfully\n");
                    }
                    else if (strcmp(clientRequest.arguments[0], "DIR"))
                    {
                        printf("Directory created successfully\n");
                    }
                }
                else if (strcmp(clientRequest.command, "DELETE"))
                {
                    if (strcmp(clientRequest.arguments[0], "FILE"))
                    {
                        printf("File deleted successfully\n");
                    }
                    else if (strcmp(clientRequest.arguments[0], "DIR"))
                    {
                        printf("Directory deleted successfully\n");
                    }
                }
            }
            // pthread_create(&operation_thread, NULL, perform_operation_NM, (void *)&response);
            // pthread_join(operation_thread, NULL);
        }
        else if (response.operation_performer == 2)
        {
            pthread_create(&operation_thread, NULL, perform_operation_ss, (void *)&response);
            pthread_join(operation_thread, NULL);
        }
        printf("Ticks: %d\n", ticks);
        ticks++;
    }

    pthread_exit(NULL);
}

int initiateClient()
{
    int socket_client;
    struct sockaddr_in addr;
    socklen_t size_address;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_client < 0)
    {
        perror("[-]Socket error");
        close(socket_client);
        exit(1);
    }

    printf("[+]Client %s socket created.\n", client.clientName);

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LEADER_SERVER_PORT_CLIENT);
    addr.sin_addr.s_addr = inet_addr(LEADER_SERVER_IP);

    size_address = sizeof(addr);

    //* Initialize client
    strcpy(client.ipAddress, CLIENT_IP);
    client.isConnected = 0;
    client.sessionID = 0;
    client.clientID = -1;

    if (connect(socket_client, (struct sockaddr *)&addr, size_address) == 0)
    {
        printf("[+]Connected to Naming-Server.\n");
    }

    int bytes_sent, bytes_received;
    bytes_sent = send(socket_client, &client, sizeof(client), 0);

    if (bytes_sent == -1)
    {
        perror("Error sending request to Naming-Server");
        close(socket_client);
        printf("[-]Connection to Naming-Server lost.\n");
    }

    bytes_received = recv(socket_client, &client, sizeof(client), 0);
    if (bytes_received == -1)
    {
        perror("Error receiving response from Naming-Server");
        close(socket_client);
        printf("[-]Connection to Naming-Server lost.\n");
    }
    else if (bytes_received == 0)
    {
        printf("Connection closed by the Naming-Server.\n");
        close(socket_client);
        printf("[-]Connection to Naming-Server lost.\n");
    }
    print_client_info(client);
    return 0;
}

int main()
{
    char temp[MAX_USERNAME_SIZE];
    printf("Enter client name: ");
    scanf("%s", temp);
    strcpy(client.clientName, temp);
    pthread_t naming_server_thread;

    initiateClient();
    flushInputBuffer();

    pthread_create(&naming_server_thread, NULL, naming_server_connection, NULL);
    pthread_join(naming_server_thread, NULL);
}