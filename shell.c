#include "include/common.h"

void custom_read(char *filepath)
{
    printf("reading from %s\n", filepath);
}

void custom_write(char *filepath)
{
    printf("writing to %s\n", filepath);
}

void custom_delete(char *filepath)
{
    printf("deleting %s\n", filepath);
}

void custom_copy(char *srcpath, char *destpath)
{
    printf("copying %s to %s\n", srcpath, destpath);
}

void custom_create(char *filepath)
{
    printf("creating %s\n", filepath);
}

void custom_fileinfo(char *filepath)
{
    printf("file information for %s\n", filepath);
}

// check if path is invalid
int invalidPath(char *path)
{
    if (access(path, F_OK) == -1)
    {
        printf("Invalid path: %s\n", path);
        return 1;
    }
    return 0;
}

// takes the input string, validates it, and calls the appropriate function
void inputCheck(char *input)
{
    char *command;
    char *args;

    command = strtok(input, " ");
    if (command == NULL)
    {
        printf("Invalid command\n");
        return;
    }

    char *filepath = strtok(NULL, " ");
    if (filepath == NULL)
    {
        printf("Invalid command: Too less arguments\n");
        return;
    }
    if (invalidPath(filepath))
        return;

    if (strcmp(command, "READ") == 0)
        custom_read(filepath);
    else if (strcmp(command, "WRITE") == 0)
        custom_write(filepath);
    else if (strcmp(command, "DELETE") == 0)
        custom_delete(filepath);
    else if (strcmp(command, "COPY") == 0)
    {
        char *srcpath, *destpath;
        strcpy(srcpath, filepath);

        destpath = strtok(NULL, " ");
        if (destpath == NULL)
        {
            printf("Invalid command: Too less arguments\n");
            return;
        }
        if (invalidPath(destpath))
            return;

        custom_copy(srcpath, destpath);
    }
    else if (strcmp(command, "CREATE") == 0)
        custom_create(filepath);
    else if (strcmp(command, "FILEINFO") == 0)
        custom_fileinfo(filepath);
    else
        printf("Invalid command\n");
}

int main()
{
    char input[MAXINPUTLENGTH];

    while (fgets(input, MAXINPUTLENGTH, stdin))
    {
        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        inputCheck(input); // parse the input and call the appropriate function
    }
}