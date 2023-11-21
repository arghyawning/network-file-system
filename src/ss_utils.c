#include "../include/common.h"

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

int createDirectory(const char *path, struct NM_to_SS_Response *response)
{
    int count = 0;
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
                return -1;
            }
            else
            {
                strcpy(response->dir[count].name, currentPath);
                count++;
                printf("Created directory: %s\n", currentPath);
            }
        }

        token = strtok(NULL, "/");
    }
    response->new_dir_count = count;
    free(copy);
    return 1;
}

int createFilePath(const char *filePath, struct NM_to_SS_Response *response)
{
    char *pathCopy = strdup(filePath);
    pathCopy = get_substring_before_last_slash(pathCopy);
    createDirectory(pathCopy, response);

    FILE *file = fopen(filePath, "w");
    if (file != NULL)
    {
        fclose(file);
        printf("File created: %s\n", filePath);
    }
    else
    {
        perror("Error creating file");
        return -1;
    }
    return 1;
}

long get_file_size(FILE *fp)
{
    fseek(fp, 0, SEEK_END);     // Move the file pointer to the end of the file
    long file_size = ftell(fp); // Get the position of the file pointer, which is the file size

    return file_size;
}