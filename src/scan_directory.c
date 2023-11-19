#include "../include/common.h"

int i, j;
int fileCount;
int dirCount;

void readDirectoryRecursive(const char *basePath, struct FileInfo **files, struct DirectoryInfo **directories)
{
    DIR *dir;
    struct dirent *ent;
    struct stat statbuf;

    // Open the directory
    if ((dir = opendir(basePath)) != NULL)
    {
        // Count the number of files and directories in the directory
        while ((ent = readdir(dir)) != NULL)
        {
            char filePath[512]; // Adjust the size according to your needs
            snprintf(filePath, sizeof(filePath), "%s/%s", basePath, ent->d_name);

            if (stat(filePath, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    fileCount++;
                }
                else if (S_ISDIR(statbuf.st_mode) && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    dirCount++;
                }
            }
        }

        // Allocate memory for the arrays of FileInfo and DirectoryInfo structs
        *files = (struct FileInfo *)realloc(*files, fileCount * sizeof(struct FileInfo));
        *directories = (struct DirectoryInfo *)realloc(*directories, dirCount * sizeof(struct DirectoryInfo));

        // Reset directory stream to the beginning
        rewinddir(dir);

        while ((ent = readdir(dir)) != NULL)
        {
            char filePath[MAX_PATH_SIZE]; // Adjust the size according to your needs
            snprintf(filePath, sizeof(filePath), "%s/%s", basePath, ent->d_name);

            if (stat(filePath, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    snprintf((*files)[i].name, sizeof((*files)[i].name), "%s", filePath);
                    i++;
                }
                else if (S_ISDIR(statbuf.st_mode) && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    snprintf((*directories)[j].name, sizeof((*directories)[j].name), "%s", filePath);
                    j++;

                    // Recursively call readDirectoryRecursive for subdirectories
                    readDirectoryRecursive(filePath, files, directories);
                }
            }
        }

        // Close the directory
        closedir(dir);
    }
    else
    {
        perror("Error opening directory");
    }
}

void scan_dir(struct FileInfo **files, struct DirectoryInfo **directories, struct NumberOfFiles *fileInfo)
{
    i = 0;
    j = 0;
    fileCount = 0;
    dirCount = 0;

    // Read the current directory
    readDirectoryRecursive(".", files, directories);

    fileInfo->numberOfFiles = fileCount;
    fileInfo->numberOfDirectories = dirCount;
}
