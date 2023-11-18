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
    char *buffer = NULL; // Initialize buffer pointer
    size_t size = 0;     // Initialize size of the buffer
    size_t index = 0;    // Initialize index for tracking the position in the buffer

    printf("Enter characters (Ctrl+D to end on Unix/Linux, Ctrl+Z on Windows):\n");

    int ch;
    while ((ch = getchar()) != EOF && ch != '\n')
    {
        if (index == size)
        {
            // If the buffer is full, reallocate to double the size
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

    // Free the allocated memory
    free(buffer);

    return 0;
}