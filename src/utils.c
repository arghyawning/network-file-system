#include "../include/common.h"

typedef struct
{
    uint64_t transactionId;
    // Other fields representing the message data
} RpcMessage;

uint64_t generateTransactionId(uint32_t clientId)
{
    //* Using a combination of timestamp, client ID, and randomness
    srandom(time(NULL));
    uint64_t timestamp = (uint64_t)time(NULL);
    uint64_t randomness = random();
    uint64_t transactionId = (timestamp << 32) | (clientId << 16) | randomness;

    return transactionId;
}

int main()
{

    // Generate a transaction ID
    uint64_t transactionId = generateTransactionId(1);

    // Create a message
    RpcMessage message;
    message.transactionId = transactionId;

    // Send the message to the server
    // ...
    printf("%llu\n", message.transactionId);

    //* print time
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;

    printf("%ld\n", time_in_micros);

    return 0;
}