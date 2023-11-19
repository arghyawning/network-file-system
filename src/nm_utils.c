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