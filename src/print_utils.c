#include "../include/common.h"

void print_response_info(struct Client_to_NM_response response)
{
    printf("Transaction ID: %llu\n", response.transactionId);
    printf("Operation Performer: %d\n", response.operation_performer);
    printf("File Path: %s\n", response.file.name);
    printf("Operation Status: %d\n", response.operation_status);
    if (response.operation_performer == 2)
    {
        printf("Storage Server Port: %d\n", response.ss_port);
        printf("Storage Server ID: %s\n", response.ss_ip);
    }
}

void print_ss_info(struct StorageServerInfo *ss, struct CombinedFilesInfo combinedFilesInfo)
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
    for (int i = 0; i < MAX_REDUNDANT_SS; i++)
    {
        printf("Redundant SS %d: %d\n", i + 1, ss->redundant_ss[i]);
    }
    //* print files and directory information
    for (int i = 0; i < ss->numberOfFiles; i++)
    {
        printf("File %d: %s\n", i + 1, combinedFilesInfo.files[i].name);
    }
    for (int i = 0; i < ss->numberOfDirectories; i++)
    {
        printf("Directory %d: %s\n", i + 1, combinedFilesInfo.directories[i].name);
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
    printf("Transaction ID: %lu\n", client_request.transactionId);
    printf("Operation: %s\n", client_request.command);
    printf("Operation Status: %d\n", client_request.operation_status);
    printf("Argument 1: %s\n", client_request.arguments[0]);
    printf("Argument 2: %s\n", client_request.arguments[1]);
}

void print_client_request_info_ss(struct Client_to_SS_Request *request)
{
    printf("Operation: %s\n", request->command);
    printf("File Name: %s\n", request->file.name);
    printf("Buffer: %s\n", request->buffer);

    return;
}

void logMessage(char *filepath, char *message)
{
    //* get time of day
    char temp_message[2048];
    time_t currentTime;
    time(&currentTime);

    // Format the time without newline character
    struct tm *localTime = localtime(&currentTime);
    char formattedTime[50]; // You can adjust the size as needed

    strftime(formattedTime, sizeof(formattedTime), "%a %b %d %Y %H:%M:%S", localTime);

    // Print the formatted time
    printf("%s\n", formattedTime);

    FILE *logFile = fopen(filepath, "a");
    if (logFile == NULL)
    {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    sprintf(temp_message, "[%s] : %s", formattedTime, message);

    fprintf(logFile, "\n%s\n", temp_message);
    fclose(logFile);
}