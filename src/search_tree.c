#include "../include/common.h"

void initialiseDirTree(struct DirTree *dirTree)
{
    dirTree->numberOfFiles = 0;
    dirTree->numberOfDirectories = 0;
    dirTree->directories = NULL;
    dirTree->files = NULL;
}

void fillDirTree(struct DirTree *dirTree, struct CombinedFilesInfo *combinedFilesInfoAll)
{
    strcpy(dirTree->dirinfo.name, ".");

    char *dirpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *nextdirpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *filepath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *nextfilepath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));

    int i, j, k;
    for (i = 0; i < MAX_SS; i++)
    {
        if (combinedFilesInfoAll[i].storageServerID == -1)
            break;

        struct DirTree *temp = dirTree;

        // adding each directory in the storage server to the tree
        for (j = 0; j < combinedFilesInfoAll[i].numberOfDirectories; j++)
        {
            char *tempdirpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
            strcpy(tempdirpath, combinedFilesInfoAll[i].directories[j].name);
            dirpath = strtok(tempdirpath, "/");
            nextdirpath = strtok(NULL, "/");

            while (nextdirpath != NULL)
            {
                // checking if the directory already exists
                int flag = 0;
                for (k = 0; k < temp->numberOfDirectories; k++)
                {
                    if (strcmp(temp->directories[k].dirinfo.name, nextdirpath) == 0)
                    {
                        temp = &temp->directories[k];
                        flag = 1;
                        break;
                    }
                }

                // if directory does not exist, create a new one
                if (flag == 0)
                {
                    temp->numberOfDirectories++;
                    temp->directories = (struct DirTree *)realloc(temp->directories, temp->numberOfDirectories * sizeof(struct DirTree));
                    initialiseDirTree(&temp->directories[temp->numberOfDirectories - 1]);
                    temp = &temp->directories[temp->numberOfDirectories - 1];
                    temp->ssid = combinedFilesInfoAll[i].storageServerID;
                    strcpy(temp->dirinfo.name, nextdirpath);
                }

                nextdirpath = strtok(NULL, "/");
            }
            temp = dirTree;
        }

        // adding each file in the storage server to the tree
        for (j = 0; j < combinedFilesInfoAll[i].numberOfFiles; j++)
        {
            temp = dirTree;
            char* tempfilepath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
            strcpy(tempfilepath, combinedFilesInfoAll[i].files[j].name);
            filepath = strtok(tempfilepath, "/");
            filepath = strtok(NULL, "/");
            nextfilepath = strtok(NULL, "/");

            // if (nextfilepath == NULL) // this is the file
            // {
            //     temp->numberOfFiles++;
            //     temp->files = (struct FileInfo *)realloc(temp->files, temp->numberOfFiles * sizeof(struct FileInfo));
            //     strcpy(temp->files[temp->numberOfFiles - 1].name, filepath);
            //     continue;
            // }

            while (nextfilepath != NULL)
            {
                // the directory should already exist because of code written above to add directories first
                int flag = 0;
                for (k = 0; k < temp->numberOfDirectories; k++)
                {
                    if (strcmp(temp->directories[k].dirinfo.name, filepath) == 0)
                    {
                        temp = &temp->directories[k];
                        flag = 1;
                        break;
                    }
                }

                // if directory does not exist, create a new one
                if (flag == 0)
                {
                    printf("Error: Directory %s does not exist\n", nextdirpath);
                    exit(1);
                    // temp->numberOfDirectories++;
                    // temp->directories = (struct DirTree *)realloc(temp->directories, temp->numberOfDirectories * sizeof(struct DirTree));
                    // initialiseDirTree(&temp->directories[temp->numberOfDirectories - 1]);
                    // temp = &temp->directories[temp->numberOfDirectories - 1];
                    // strcpy(temp->dirinfo.name, nextdirpath);
                }

                strcpy(filepath, nextfilepath);
                nextfilepath = strtok(NULL, "/");
            }

            // add the file to the directory
            temp->numberOfFiles++;
            temp->files = (struct FileInfo *)realloc(temp->files, temp->numberOfFiles * sizeof(struct FileInfo));
            temp->ssid = combinedFilesInfoAll[i].storageServerID;
            strcpy(temp->files[temp->numberOfFiles - 1].name, filepath);
        }
    }

    // if(dirpath != NULL)
    //     free(dirpath);
    if (nextdirpath != NULL)
        free(nextdirpath);
    // if(filepath != NULL)
    //     free(filepath);
    // if(nextfilepath != NULL)
    //     free(nextfilepath);
}

int searchDirTree(struct DirTree *dirTree, char *filename)
{

    if (dirTree == NULL)
        return -1;

    char *path = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *nextpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *searchfile = (char *)malloc(MAX_PATH_SIZE * sizeof(char));

    char *temp = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(temp, filename);

    searchfile = strrchr(temp, '/') + 1;
    printf("1. Searching for file '%s'\n", searchfile);

    strcpy(temp, filename);
    path = strtok(temp, "/");
    printf("1. %s\n", path);
    path = strtok(NULL, "/");
    printf("2. %s\n", path);
    // strcpy(searchfile, path);
    if (path == NULL)
    {
        printf("1. Error: Invalid path\n");
        return -1;
    }
    nextpath = strtok(NULL, "/");

    while (nextpath != NULL)
    {
        int flag = 0;
        int i;
        for (i = 0; i < dirTree->numberOfDirectories; i++)
        {
            if (strcmp(dirTree->directories[i].dirinfo.name, path) == 0)
            {
                dirTree = &dirTree->directories[i];
                flag = 1;
                break;
            }
        }

        if (flag == 0)
        {
            printf("Error: Directory %s does not exist\n", path);
            return -1;
        }

        strcpy(path, nextpath);
        nextpath = strtok(NULL, "/");
    }

    int i;
    for (i = 0; i < dirTree->numberOfFiles; i++)
    {
        printf("Comparing %s and %s\n", dirTree->files[i].name, searchfile);
        if (strcmp(dirTree->files[i].name, searchfile) == 0)
        {
            printf("File %s found in directory %s\n", searchfile, dirTree->dirinfo.name);
            return dirTree->ssid;
        }
    }

    printf("File %s not found\n", searchfile);
    return -1;
}

struct TreeClosestDirPacket closestDir(struct DirTree *dirTree, char *filename)
{
    struct TreeClosestDirPacket treeClosestDirPacket;
    treeClosestDirPacket.ssid = -1;
    strcpy(treeClosestDirPacket.dirinfo.name, ".");

    if (dirTree == NULL)
        return treeClosestDirPacket;

    char *path = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *nextpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));

    char *currpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(currpath, ".");

    char *temp = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(temp, filename);

    struct DirTree *curr = dirTree;

    int i, depth = 0, maxdepth = 0;

    path = strtok(temp, "/");
    path = strtok(NULL, "/");
    nextpath = strtok(NULL, "/");
    depth = 0;

    while (nextpath != NULL)
    {
        int flag = 0;
        for (i = 0; i < curr->numberOfDirectories; i++)
        {
            if (strcmp(curr->directories[i].dirinfo.name, path) == 0)
            {
                curr = &curr->directories[i];
                flag = 1;
                break;
            }
        }

        if (flag == 0)
            return treeClosestDirPacket;

        strcpy(path, nextpath);
        nextpath = strtok(NULL, "/");
        depth++;

        strcat(currpath, "/");
        strcat(currpath, curr->dirinfo.name);

        if (depth > maxdepth)
        {
            maxdepth = depth;
            treeClosestDirPacket.ssid = curr->ssid;
            // strcpy(treeClosestDirPacket.dirinfo.name, curr->dirinfo.name);
            strcpy(treeClosestDirPacket.dirinfo.name, currpath);
        }
    }

    return treeClosestDirPacket;
}

int removeFromTree(struct DirTree *dirTree, int type, char *delpath)
{
    // type 0 for file, 1 for directory

    if (dirTree == NULL)
    {
        printf("Error: Path %s does not exist\n", delpath);
        return 0;
    }

    char *path = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    char *nextpath = (char *)malloc(MAX_PATH_SIZE * sizeof(char));

    char *temp = (char *)malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(temp, delpath);

    path = strtok(temp, "/");
    path = strtok(NULL, "/");
    if (path == NULL)
    {
        printf("Error: Path %s does not exist\n", delpath);
        return 0;
    }
    nextpath = strtok(NULL, "/");

    while (nextpath != NULL)
    {
        int flag = 0;
        int i;
        for (i = 0; i < dirTree->numberOfDirectories; i++)
        {
            if (strcmp(dirTree->directories[i].dirinfo.name, path) == 0)
            {
                dirTree = &dirTree->directories[i];
                flag = 1;
                break;
            }
        }

        if (flag == 0)
        {
            printf("Error: Directory %s does not exist\n", path);
            return 0;
        }

        strcpy(path, nextpath);
        nextpath = strtok(NULL, "/");
    }

    int i;
    if (type == 0) // file
    {
        for (i = 0; i < dirTree->numberOfFiles; i++)
        {
            if (strcmp(dirTree->files[i].name, path) == 0)
            {
                int j;
                for (j = i; j < dirTree->numberOfFiles - 1; j++)
                    dirTree->files[j] = dirTree->files[j + 1];
                dirTree->numberOfFiles--;
                dirTree->files = (struct FileInfo *)realloc(dirTree->files, dirTree->numberOfFiles * sizeof(struct FileInfo));
                return 1;
            }
        }
    }
    else // directory
    {
        for (i = 0; i < dirTree->numberOfDirectories; i++)
        {
            if (strcmp(dirTree->directories[i].dirinfo.name, path) == 0)
            {
                int j;
                dirTree->directories[i].directories = NULL;
                dirTree->directories[i].files = NULL;
                dirTree->directories[i].numberOfDirectories = 0;
                dirTree->directories[i].numberOfFiles = 0;
                
                for (j = i; j < dirTree->numberOfDirectories - 1; j++)
                    dirTree->directories[j] = dirTree->directories[j + 1];
                dirTree->numberOfDirectories--;
                dirTree->directories = (struct DirTree *)realloc(dirTree->directories, dirTree->numberOfDirectories * sizeof(struct DirTree));
                return 1;
            }
        }
    }
    return 0;
}

// int main()
// {
//     struct CombinedFilesInfo combinedFilesInfoAll[MAX_SS];

//     fillFileInfo();

//     struct DirTree dirTree;
//     initialiseDirTree(&dirTree);
//     fillDirTree(&dirTree, combinedFilesInfoAll);

//     int ssid = searchDirTree(&dirTree, "./delete.txt");

//     return 0;
// }