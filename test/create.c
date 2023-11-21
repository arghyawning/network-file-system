#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define CHUNK_SIZE 1024

char logFilePath[256] = ".nm_log";

void logMessage(char *message)
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


    FILE *logFile = fopen(logFilePath, "a");
    if (logFile == NULL)
    {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    sprintf(temp_message, "[%s] : %s", formattedTime, message);

    fprintf(logFile, "\n%s\n", temp_message);
    fclose(logFile);
}

int main()
{
    logMessage("Starting Naming Server");

    return 0;
}
