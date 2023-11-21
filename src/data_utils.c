#include "../include/common.h"

//* Deserialize the char buffer into StorageServerInfo, DirectoryInfo and FileInfo
struct CombinedFilesInfo deserializeData(char *buffer, struct StorageServerInfo *ss)
{
    int nFiles, nDirectories;

    //* Deserialize StorageServerInfo
    memcpy(ss, buffer, sizeof(struct StorageServerInfo));
    nFiles = ss->numberOfFiles;
    nDirectories = ss->numberOfDirectories;

    //* Calculate the offset for DirectoryInfo
    size_t offset = sizeof(struct StorageServerInfo);

    //* Deserialize DirectoryInfo
    struct DirectoryInfo *directories_all = (struct DirectoryInfo *)malloc(sizeof(struct DirectoryInfo) * nDirectories);
    memcpy(directories_all, buffer + offset, sizeof(struct DirectoryInfo) * nDirectories);

    // *Calculate the new offset for FileInfo
    offset += sizeof(struct DirectoryInfo) * nDirectories;

    //* Deserialize FileInfo
    struct FileInfo *files_all = (struct FileInfo *)malloc(sizeof(struct FileInfo) * nFiles);
    memcpy(files_all, buffer + offset, sizeof(struct FileInfo) * nFiles);

    struct CombinedFilesInfo combinedFilesInfo;
    combinedFilesInfo.numberOfFiles = ss->numberOfFiles;
    combinedFilesInfo.numberOfDirectories = ss->numberOfDirectories;
    combinedFilesInfo.files = files_all;
    combinedFilesInfo.directories = directories_all;

    return combinedFilesInfo;
}

//* Serialize the the StorageServerInfo, DirectoryInfo and FileInfo into a char buffer to send in a single send() call
void serializeData(struct StorageServerInfo *ss, int nFiles, int nDirectories, struct DirectoryInfo directories_all[], struct FileInfo files_all[], char *buffer)
{
    // print_ss_info(ss);
    memcpy(buffer, ss, sizeof(struct StorageServerInfo));
    // printf("Buffer: %s\n", buffer);
    // Calculate the offset for DirectoryInfo
    size_t offset = sizeof(struct StorageServerInfo);
    // printf("Offset: %d\n", offset);
    // Serialize DirectoryInfo
    memcpy(buffer + offset, directories_all, sizeof(struct DirectoryInfo) * nDirectories);
    // printf("Buffer: %s\n", buffer);
    // Calculate the new offset for FileInfo
    offset += sizeof(struct DirectoryInfo) * nDirectories;
    // printf("Offset: %d\n", offset);
    // Serialize FileInfo
    memcpy(buffer + offset, files_all, sizeof(struct FileInfo) * nFiles);

    // printf("Buffer: %s\n", buffer);
}

//* strings operations
char *get_substring_before_last_slash(const char *path) {
    const char *last_slash = strrchr(path, '/');

    if (last_slash == NULL) {
        return NULL;  // No '/' in the string
    }

    // Calculate the length of the substring before the last '/'
    size_t length = last_slash - path;

    // Allocate memory for the substring
    char *substring = malloc(length + 1);  // +1 for null terminator

    if (substring == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Copy the substring before the last '/' into the new buffer
    strncpy(substring, path, length);
    substring[length] = '\0';  // Null-terminate the substring

    return substring;
}