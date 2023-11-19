#include "../include/common.h"

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
