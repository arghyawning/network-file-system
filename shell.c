#include "include/common.h"

void custom_read(char *filepath)
{
    // printf("reading from %s\n", filepath);

    FILE *fp;
    char ch;
    fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        printf("Cannot open file \n");
        return;
    }

    // Read contents from file
    ch = fgetc(fp);
    while (ch != EOF)
    {
        printf("%c", ch);
        ch = fgetc(fp);
    }

    fclose(fp);

    printf("\n");
}

void custom_write(char *filepath)
{
    // printf("writing to %s\n", filepath);

    char data[16384];
    printf("Enter the data to be written into the file: \n");
    scanf("%[^\n]%*c", data);

    printf("1. Overwrite\n2. Append\n");
    int choice;
    scanf("%d", &choice);

    FILE *fp;
    if (choice == 1)
        fp = fopen(filepath, "w");
    else if (choice == 2)
        fp = fopen(filepath, "a");
    else
    {
        printf("Invalid choice\n");
        return;
    }

    if (fp == NULL)
    {
        printf("Cannot open file \n");
        return;
    }

    // Write data to file
    fputs(data, fp);

    fclose(fp);

    printf("Data written successfully.\n");
}

void custom_delete(char *filepath)
{
    // printf("deleting %s\n", filepath);

    if (remove(filepath) == 0)
        printf("Deleted successfully\n");
    else
        printf("Unable to delete the file\n");
}

void custom_copy(char *srcpath, char *destpath)
{
    // printf("copying %s to %s\n", srcpath, destpath);

    FILE *fpsrc, *fpdest;
    char *destfile, *filename;
    char c;

    fpsrc = fopen(srcpath, "r");
    if (fpsrc == NULL)
    {
        printf("Cannot open file \n");
        return;
    }

    // Get the filename from the source file path
    filename = strrchr(srcpath, '/');
    if (filename == NULL)
        filename = srcpath;

    // Construct the destination file path
    destfile = (char *)malloc(strlen(destpath) + strlen(filename) + 2);
    strcpy(destfile, destpath);
    strcat(destfile, "/");
    strcat(destfile, filename);

    fpdest = fopen(destfile, "w");
    if (fpdest == NULL)
    {
        printf("Cannot open file \n");
        return;
    }

    // Read contents from file
    c = fgetc(fpsrc);
    while (c != EOF)
    {
        fputc(c, fpdest);
        c = fgetc(fpsrc);
    }

    fclose(fpsrc);
    fclose(fpdest);

    printf("File copied successfully.\n");

    free(destfile);
}

void custom_create(char *filepath)
{
    // printf("creating %s\n", filepath);

    FILE *fp;
    fp = fopen(filepath, "w");
    if (fp == NULL)
    {
        printf("Cannot create file \n");
        return;
    }

    fclose(fp);

    printf("File created successfully.\n");
}

void custom_fileinfo(char *filepath)
{
    // printf("file information for %s\n", filepath);

    struct stat fileStat;
    if (stat(filepath, &fileStat) < 0)
    {
        printf("Cannot get file information \n");
        return;
    }

    printf("Information for %s\n", filepath);
    printf("---------------------------\n");
    printf("File Size: \t\t%ld bytes\n", fileStat.st_size);
    printf("File Mode: \t\t%d\n", fileStat.st_mode);
    printf("Number of Links: \t%ld\n", fileStat.st_nlink);
    printf("File inode: \t\t%ld\n", fileStat.st_ino);
    printf("File UID: \t\t%d\n", fileStat.st_uid);
    printf("File GID: \t\t%d\n", fileStat.st_gid);
    printf("File Blocks: \t\t%ld\n", fileStat.st_blocks);
    printf("Last Accessed: \t\t%s", ctime(&fileStat.st_atime));
    printf("Last Modified: \t\t%s", ctime(&fileStat.st_mtime));
    printf("Last Status Change: \t%s", ctime(&fileStat.st_ctime));

    // Print file permissions
    printf("File Permissions: ");

    // Owner permissions
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");

    // Group permissions
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");

    // Others permissions
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf("\n");
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

// error display
void errorDisplay(int flag)
{
    switch (flag)
    {
    case 1:
        printf("No input entered\n");
        break;
    case 2:
        printf("Invalid command: Too less arguments\n");
        break;
    case 3:
        printf("Invalid command\n");
        break;
    default:
        printf("Error!\n");
        break;
    }
}

// takes the input string, validates it, and calls the appropriate function
void inputCheck(char *input)
{
    char *command;
    char *args;

    command = strtok(input, " ");
    if (command == NULL)
    {
        errorDisplay(1);
        return;
    }

    char *filepath = strtok(NULL, " ");

    if (strcmp(command, "READ") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        if (invalidPath(filepath))
            return;

        custom_read(filepath);
    }
    else if (strcmp(command, "WRITE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        if (invalidPath(filepath))
            return;

        custom_write(filepath);
    }
    else if (strcmp(command, "DELETE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        if (invalidPath(filepath))
            return;

        custom_delete(filepath);
    }
    else if (strcmp(command, "COPY") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }
        if (invalidPath(filepath))
            return;

        char *srcpath, *destpath;
        strcpy(srcpath, filepath);

        destpath = strtok(NULL, " ");
        if (destpath == NULL)
        {
            errorDisplay(2);
            return;
        }
        if (invalidPath(destpath))
            return;

        custom_copy(srcpath, destpath);
    }
    else if (strcmp(command, "CREATE") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        custom_create(filepath);
    }
    else if (strcmp(command, "FILEINFO") == 0)
    {
        if (filepath == NULL)
        {
            errorDisplay(2);
            return;
        }

        if (invalidPath(filepath))
            return;

        custom_fileinfo(filepath);
    }
    else
        errorDisplay(3);
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

    return 0;
}