#include "../include/common.h"

//* function to assign ports to clients
void assign_ports_client(struct ClientInfo *client)
{
    int client_id = client->clientID;
    int base_port = BASE_PORT_CLIENT + client_id * MAX_PORTS_PER_CLIENT;
    client->clientPort = base_port;

    return;
}

//* function to assign ports to SS
void assign_ports_ss(struct StorageServerInfo *ss)
{
    int ss_id = ss->storageServerID;
    int base_port = BASE_PORT_SS + ss_id * MAX_PORTS_PER_SS;
    ss->heartbeatPort = base_port;
    ss->storageServerPort = base_port + 1;
    ss->clientPort = base_port + 2;
    ss->copyPort = base_port + 3;
    return;
}

int compareFilePath(const char *X, const struct StorageServerInfo *array, int number_of_ss)
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

void createFileFromArray(const char *filename, struct CombinedFilesInfo *combinedFilesInfoAll, struct StorageServerInfo *storageServers, int number_of_ss)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    printf("Writing to accessible paths to the file: %s\n", filename);

    //* First write all the files
    fprintf(file, "Accessible Files:\n");
    for (int j = 0; j < number_of_ss; j++)
    {
        if (storageServers[j].isConnected == 1)
        {
            for (int i = 0; i < combinedFilesInfoAll[j].numberOfFiles; i++)
            {
                fprintf(file, "%s\n", combinedFilesInfoAll[j].files[i].name);
                printf("%s\n", combinedFilesInfoAll[j].files[i].name);
            }
        }
    }

    //* Write all the directories
    fprintf(file, "\nAccessible Directories:\n");
    for (int j = 0; j < number_of_ss; j++)
    {
        if (storageServers[j].isConnected == 1)
        {
            for (int i = 0; i < combinedFilesInfoAll[j].numberOfDirectories; i++)
            {
                fprintf(file, "%s\n", combinedFilesInfoAll[j].directories[i].name);
                printf("%s\n", combinedFilesInfoAll[j].directories[i].name);

            }
        }
    }
    printf("Done writing to the file: %s\n", filename);

    fclose(file);
}

void sendFile(int client_socket, const char *filename)
{
    FILE *fp = fopen(filename, "r");
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

        if (send(client_socket, &packet, sizeof(struct Packet), 0) == -1)
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
    if (send(client_socket, &packet, sizeof(struct Packet), 0) == -1)
    {
        perror("[-] Error in sending data");
        exit(1);
    }

    fclose(fp);
}