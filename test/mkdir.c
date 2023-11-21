#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char *get_substring_before_last_slash(const char *path)
{
    const char *last_slash = strrchr(path, '/');

    if (last_slash == NULL)
    {
        return NULL; // No '/' in the string
    }

    // Calculate the length of the substring before the last '/'
    size_t length = last_slash - path;

    // Allocate memory for the substring
    char *substring = malloc(length + 1); // +1 for null terminator

    if (substring == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Copy the substring before the last '/' into the new buffer
    strncpy(substring, path, length);
    substring[length] = '\0'; // Null-terminate the substring

    return substring;
}

void removeConsecutiveDots(char *path)
{
    int len = strlen(path);
    int i, j;

    for (i = 0, j = 0; i < len; i++)
    {
        if (path[i] == '.' && path[i + 1] == '/' && path[i + 2] == '.')
        {
            // Skip the consecutive "./"
            i += 1;
        }
        else
        {
            // Copy the character to the new string
            path[j++] = path[i];
        }
    }

    // Null-terminate the modified string
    path[j] = '\0';
}

void createDirectory(const char *path)
{
    char *copy = strdup(path);
    char *token = strtok(copy, "/");
    char currentPath[256] = "."; // Assuming a maximum path length of 256 characters

    while (token != NULL)
    {
        strcat(currentPath, "/");
        strcat(currentPath, token);
        removeConsecutiveDots(currentPath);
        // Check if the directory already exists
        struct stat st;
        if (stat(currentPath, &st) != 0)
        {
            // Directory does not exist, create it
            if (mkdir(currentPath, 0777) != 0)
            {
                fprintf(stderr, "Error creating directory %s\n", currentPath);
                free(copy);
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("Created directory: %s\n", currentPath);
            }
        }

        token = strtok(NULL, "/");
    }

    free(copy);
}

void createFilePath(const char *filePath)
{
    char *pathCopy = strdup(filePath);
    pathCopy = get_substring_before_last_slash(pathCopy);
    createDirectory(pathCopy);

    FILE *file = fopen(filePath, "w");
    if (file != NULL)
    {
        fclose(file);
        printf("File created: %s\n", filePath);
    }
    else
    {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    const char *filePath = "path/to/your/file/file.txt";

    createFilePath(filePath);

    printf("File created successfully: %s\n", filePath);

    return 0;
}
