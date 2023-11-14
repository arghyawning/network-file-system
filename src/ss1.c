// storage_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LEADER_SERVER_IP "127.0.0.1"
#define LEADER_SERVER_PORT 12345
#define CLIENT_PORT 5678
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
void sendInformationToLeaderServer();
void sendHeartbeat();

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

    return 0;
}

void* heartbeatThread(void* arg) {
    while (1) {
        // Implement heartbeat logic here
        pthread_mutex_lock(&mutex);

        // Send heartbeat message to leader server
        // You need to implement the actual network communication here
        // ...

        printf("Heartbeat sent to Leader Server\n");

        pthread_mutex_unlock(&mutex);

        sleep(10); // Send heartbeat every 10 seconds (adjust as needed)
    }
}

void* dataThread(void* arg) {
    while (1) {
        sendInformationToLeaderServer();

        sleep(10); // Sleep for 10 seconds (adjust as needed)
    }

    pthread_exit(NULL);
}

// Rest of the code remains the same...
