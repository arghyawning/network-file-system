// leader_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define LEADER_SERVER_PORT 12345
#define MAX_BUFFER_SIZE 1024

struct StorageServerInfo {
    char ipAddress[16];
    int leaderServerPort;
    int clientPort;
    char directory[256];
    char accessibleFiles[256];
    int isConnected;
};

void* heartbeatThread(void* arg);
void* dataThread(void* arg);
void handleHeartbeat(char* ipAddress, int isConnected);
void updateStorageServerInfo(struct StorageServerInfo* info);
void printStorageServers();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronization

int main() {
    // ... (Same initialization as before)

    // Create threads for heartbeat and data handling
    pthread_t heartbeatThreadId, dataThreadId;
    
    pthread_create(&heartbeatThreadId, NULL, heartbeatThread, NULL);
    pthread_create(&dataThreadId, NULL, dataThread, NULL);

    // Wait for threads to finish
    pthread_join(heartbeatThreadId, NULL);
    pthread_join(dataThreadId, NULL);

    // Close the server socket
    close(serverSocket);

    return 0;
}

void* heartbeatThread(void* arg) {
    while (1) {
        // Implement heartbeat logic here
        pthread_mutex_lock(&mutex);

        // Iterate through storage servers and send heartbeat messages
        for (struct StorageServerInfo* info = storageServers; info != NULL; info = info->next) {
            if (info->isConnected) {
                // Send heartbeat message to storage server
                // You need to implement the actual network communication here
                // ...

                printf("Heartbeat sent to Storage Server %s\n", info->ipAddress);
            }
        }

        pthread_mutex_unlock(&mutex);

        sleep(5); // Send heartbeat every 5 seconds (adjust as needed)
    }
}

void* dataThread(void* arg) {
    while (1) {
        // Accept incoming connections and handle data
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Accept connection from storage server
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Receive information from storage server
        struct StorageServerInfo info;
        ssize_t bytesReceived = recv(clientSocket, &info, sizeof(info), 0);
        if (bytesReceived == -1) {
            perror("Error receiving data");
            close(clientSocket);
            continue;
        } else if (bytesReceived == 0) {
            printf("Storage Server disconnected.\n");
            close(clientSocket);
            continue;
        }

        // Handle heartbeats
        handleHeartbeat(info.ipAddress, info.isConnected);

        // Update storage server information
        updateStorageServerInfo(&info);

        // Print updated storage server information
        printStorageServers();

        // Close the connection
        close(clientSocket);
    }

    pthread_exit(NULL);
}

// Rest of the code remains the same...
