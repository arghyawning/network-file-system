#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

const int PORT = 8080;
// const char *IP = "127.0.0.1";
const char *IP = "192.168.2.204";

#define CHUNK_SIZE 256
#define TIMEOUT 0.1
#define MAXPATH 256

int *acks;

time_t start;

struct datapack
{
    int index;
    char data[CHUNK_SIZE];
    int numchunks;
};

struct arguments
{
    int sock;
    struct sockaddr_in server_address;
    int numchunks;
    struct datapack *chunks;
};

void *acknowledging(void *args)
{
    struct arguments *targs = (struct arguments *)args;

    int acksnum = 0;
    socklen_t address_size = sizeof(targs->server_address);
    while (acksnum < targs->numchunks)
    {
        // printf("hi\n");
        // struct datapack dp;
        int index;
        if (recvfrom(targs->sock, &index, sizeof(int), 0, (struct sockaddr *)&targs->server_address, &address_size) < 0)
        // if (recvfrom(targs->sock, &index, sizeof(int), 0, NULL, NULL) < 0)
        {
            perror("receiving failed");
            exit(EXIT_FAILURE);
        }
        printf("[+]Ack recv: %d\n", index);
        // // printf("hi\n");
        acks[index] = 1;
        acksnum = 0;
        for (int i = 0; i < targs->numchunks; i++)
        {
            if (acks[i] == 1)
                acksnum++;
        }
    }
}

void *resending(void *args)
{
    struct arguments *targs = (struct arguments *)args;
    int acksnum = 0;
    socklen_t address_size = sizeof(targs->server_address);
    while (acksnum < targs->numchunks)
    {
        for (int i = 0; i < targs->numchunks; i++)
        {
            if (acks[i] == 0 && time(NULL) - start > TIMEOUT)
            {
                if (sendto(targs->sock, &targs->chunks[i], sizeof(targs->chunks[i]), 0, (struct sockaddr *)&targs->server_address, address_size) < 0)
                {
                    perror("sending failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
        acksnum = 0;
        for (int i = 0; i < targs->numchunks; i++)
        {
            if (acks[i] == 1)
                acksnum++;
        }
    }
}

// int chunking(char *data, struct datapack **dp)
// {
//     int numchunks = 1 + strlen(data) / CHUNK_SIZE;
//     *dp = (struct datapack *)malloc(numchunks * sizeof(struct datapack));
//     acks = (int *)calloc(numchunks, sizeof(int));

//     int i, d = 0;
//     for (i = 0; i < numchunks; i++)
//     {
//         (*dp)[i].index = i;
//         (*dp)[i].numchunks = numchunks;
//         strncpy((*dp)[i].data, data + d, CHUNK_SIZE);
//         // (*dp)[i].data[CHUNK_SIZE] = '\0';
//         // (*dp)[i].data[CHUNK_SIZE - 1] = '\0';
//         d += CHUNK_SIZE;
//     }

//     return numchunks;
// }

int chunking(FILE *fp, struct datapack **dp)
{
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int numchunks = 1 + filesize / CHUNK_SIZE;
    *dp = (struct datapack *)malloc(numchunks * sizeof(struct datapack));

    int i, bytesRead = 0;
    for (i = 0; i < numchunks; i++)
    {
        (*dp)[i].index = i;
        (*dp)[i].numchunks = numchunks;

        // Read CHUNK_SIZE bytes or remaining bytes if less than CHUNK_SIZE
        int bytesToRead = (filesize - bytesRead < CHUNK_SIZE) ? (filesize - bytesRead) : CHUNK_SIZE;
        fread((*dp)[i].data, 1, bytesToRead, fp);

        bytesRead += bytesToRead;
    }

    return numchunks;
}

int main()
{
    int sock;
    struct sockaddr_in server_address, client_address;

    // initialise udp socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // initialise server address and port
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(IP);

    // printf("Enter message to send: ");
    char filename[MAXPATH]; // assuming max file path length to be 256
    printf("Enter the absolute/relative path of the file to be sent: ");
    scanf("%s", filename);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("[-]file error");
        exit(EXIT_FAILURE);
    }

    // data input

    // chunking the data
    struct datapack *dp;
    // int numchunks = chunking(data, &dp);
    int numchunks = chunking(fp, &dp);
    printf("[+]Number of chunks: %d\n", numchunks);
    acks = (int *)calloc(numchunks, sizeof(int));

    // printing the chunks
    for (int i = 0; i < numchunks; i++)
        printf("[+]Chunk %d: %s\n", dp[i].index, dp[i].data);

    // arguments for threads
    struct arguments *targs = (struct arguments *)malloc(sizeof(struct arguments));
    targs->sock = sock;
    targs->server_address = server_address;
    targs->numchunks = numchunks;
    targs->chunks = dp;

    // creating the two threads
    pthread_t acknowledge, resend;
    pthread_create(&acknowledge, NULL, &acknowledging, targs);
    pthread_create(&resend, NULL, &resending, targs);

    // sending the data
    start = time(NULL);
    for (int i = 0; i < numchunks; i++)
    {
        if (sendto(sock, &dp[i], sizeof(dp[i]), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0)
        {
            perror("sending failed");
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(acknowledge, NULL);
    pthread_join(resend, NULL);

    // free(data);
    free(dp);
    free(acks);
    free(targs);

    close(sock);

    return 0;
}