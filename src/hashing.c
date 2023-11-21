#include "../include/common.h"

void initialize_hash_table(bucket *fileshash, bucket_dir *dirhash)
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        fileshash[i].key = i;
        dirhash[i].key = i;
        dirhash[i].num_directories = 0;
        fileshash[i].num_files = 0;
    }
}

void add_dir_in_hash(char *filename, int ssid, bucket_dir *dirhash)
{

    // char *filename_updated = strrchr(filename, '/') + 1;

    // int key = strlen(filename_updated);
    int key = strlen(filename);
    if (dirhash[key].num_directories == MAX_FILES)
    {
        printf("[-]Hash table bucket full. Cannot store more files.\n");
        return;
    }
    dd temp;
    temp.ssid = ssid;
    strcpy(temp.dirpath.name, filename);
    dirhash[key].directories[dirhash[key].num_directories].ssid = ssid;
    strcpy(dirhash[key].directories[dirhash[key].num_directories].dirpath.name, filename);
    // printf("before: number of files %d for key %d\n", dirhash[key].num_directories, key);
    dirhash[key].num_directories++;
    // printf("after: number of files %d for key %d\n", dirhash[key].num_directories, key);
}

void add_file_in_hash(char *filename, int ssid, bucket *fileshash)
{

    // char *filename_updated = strrchr(filename, '/') + 1;

    // int key = strlen(filename_updated);
    int key = strlen(filename);
    if (fileshash[key].num_files == MAX_FILES)
    {
        printf("[-]Hash table bucket full. Cannot store more files.\n");
        return;
    }
    // ff temp;
    // temp.ssid = ssid;
    // strcpy(temp.filepath.name, filename);
    // fileshash[key].files[fileshash[key].num_files] = temp;
    fileshash[key].files[fileshash[key].num_files].ssid = ssid;
    strcpy(fileshash[key].files[fileshash[key].num_files].filepath.name, filename);
    // printf("before: number of files %d for key %d\n", fileshash[key].num_files, key);
    fileshash[key].num_files++;
    //* print the bucket
    printf("Printing buckets\n\n\n\n\n");
    for (int i = 0; i < fileshash[key].num_files; i++)
    {
        printf("File %d: %s\n", i + 1, fileshash[key].files[i].filepath.name);
    }
    // printf("after: number of files %d for key %d\n", fileshash[key].num_files, key);
}

void store_in_hash_dir(struct CombinedFilesInfo *directories, bucket_dir *dirhash)
{
    for (int i = 0; i < directories->numberOfDirectories; i++)
    {
        //! Restore below if any problems
        // char *filename = strrchr(directories->directories[i].name, '/') + 1;

        //* key is the complete path length
        char *filename = directories->directories[i].name;

        int key = strlen(filename);
        if (dirhash[key].num_directories == MAX_FILES)
        {
            printf("[-]Hash table bucket full. Cannot store more files.\n");
            continue;
        }

        dirhash[key].directories[dirhash[key].num_directories] = (dd){directories->storageServerID, directories->directories[i]};
        dirhash[key].num_directories++;
    }
}

void store_in_hash_file(struct CombinedFilesInfo *files, bucket *fileshash)
{
    for (int i = 0; i < files->numberOfFiles; i++)
    {
        //! Restore below if any problems
        // char *filename = strrchr(files->files[i].name, '/') + 1;

        //* key is the complete path length
        char *filename = files->files[i].name;

        int key = strlen(filename);
        if (fileshash[key].num_files == MAX_FILES)
        {
            printf("[-]Hash table bucket full. Cannot store more files.\n");
            continue;
        }

        fileshash[key].files[fileshash[key].num_files] = (ff){files->storageServerID, files->files[i]};
        fileshash[key].num_files++;
    }
}

dd dirSearchWithHash(char *searchfilename, bucket_dir *dirhash)
{
    int key = strlen(searchfilename);
    bucket_dir b = dirhash[key];

    printf("Number of directories with key %d: %d\n", key, b.num_directories);

    for (int i = 0; i < b.num_directories; i++)
    {
        // retrieving file name from path
        // printf("Before: File %d: %s\n", i + 1, b.directories[i].dirpath.name);
        // char *filename = strrchr(b.directories[i].dirpath.name, '/') + 1;
        char *filename = b.directories[i].dirpath.name;
        // printf("After: File %d: %s\n", i + 1, filename);
        if (filename == NULL)
        {
            strcpy(filename, b.directories[i].dirpath.name);
        }

        // comparing file name with searchfilename
        if (strcmp(filename, searchfilename) == 0)
        {
            return b.directories[i];
        }
    }

    dd notfound;
    notfound.ssid = -1;
    notfound.dirpath.name[0] = '\0';
    return notfound;
}

ff fileSearchWithHash(char *searchfilename, bucket *fileshash)
{

    int key = strlen(searchfilename);
    bucket b = fileshash[key];

    printf("Number of files with key %d: %d\n", key, b.num_files);

    for (int i = 0; i < b.num_files; i++)
    {
        // retrieving file name from path
        // printf("Before: File %d: %s\n", i + 1, b.files[i].filepath.name);
        // char *filename = strrchr(b.files[i].filepath.name, '/') + 1;
        char *filename = b.files[i].filepath.name;
        // printf("After: File %d: %s\n", i + 1, filename);
        if (filename == NULL)
        {
            strcpy(filename, b.files[i].filepath.name);
        }

        // comparing file name with searchfilename
        if (strcmp(filename, searchfilename) == 0)
        {
            return b.files[i];
        }
    }

    ff notfound;
    notfound.ssid = -1;
    notfound.filepath.name[0] = '\0';
    return notfound;
}

void removeHashEntryFile(char *filename, bucket *fileshash)
{
    int key = strlen(filename);
    bucket *b = &fileshash[key];

    printf("Number of files with key %d: %d\n", key, b->num_files);

    for (int i = 0; i < b->num_files; i++)
    {
        // retrieving file name from path
        // printf("Before: File %d: %s\n", i + 1, b->files[i].filepath.name);
        char *filename_updated = b->files[i].filepath.name;
        // printf("After: File %d: %s\n", i + 1, filename_updated);
        // if (filename_updated == NULL)
        // {
        //     strcpy(filename, b->files[i].filepath.name);
        // }

        // comparing file name with searchfilename
        if (strcmp(filename_updated, filename) == 0)
        {
            // Shift elements to the left to fill the gap
            printf("deleting file %s\n", filename);

            for (int j = i; j < b->num_files - 1; j++)
            {
                b->files[j] = b->files[j + 1];
            }
            b->num_files--;
            break;
        }
    }
    printf("updated buckets\n");
    for (int i = 0; i < b->num_files; i++)
    {
        printf("File %d: %s\n", i + 1, b->files[i].filepath.name);
    }
}

void removeHashEntryDirectory(char *filename, bucket_dir *dirhash)
{
    int key = strlen(filename);
    bucket_dir *b = &dirhash[key];

    printf("Number of directories with key %d: %d\n", key, b->num_directories);

    for (int i = 0; i < b->num_directories; i++)
    {
        // retrieving file name from path
        // printf("Before: File %d: %s\n", i + 1, b->directories[i].dirpath.name);
        // char *filename_updated = strrchr(b->directories[i].dirpath.name, '/') + 1;
        char *filename_updated = b->directories[i].dirpath.name;
        // printf("After: File %d: %s\n", i + 1, filename_updated);
        // if (filename_updated == NULL)
        // {
        //     strcpy(filename, b->directories[i].dirpath.name);
        // }

        // comparing file name with searchfilename
        if (strcmp(filename_updated, filename) == 0)
        {
            // Shift elements to the left to fill the gap
            printf("deleting directory %s\n", filename);

            for (int j = i; j < b->num_directories - 1; j++)
            {
                b->directories[j] = b->directories[j + 1];
            }
            b->num_directories--;
            break;
        }
    }
    printf("updated buckets\n");
    for (int i = 0; i < b->num_directories; i++)
    {
        printf("Directory %d: %s\n", i + 1, b->directories[i].dirpath.name);
    }
}

void print_hash_table_files(bucket *fileshash)
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        bucket b = fileshash[i];
        printf("Key: %d\n", b.key);
        printf("Number of files: %d\n", b.num_files);
        for (int j = 0; j < b.num_files; j++)
        {
            printf("File %d: %s\n", j + 1, b.files[j].filepath.name);
        }
    }
}

void print_hash_table_directories(bucket_dir *dirhash)
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        bucket_dir b = dirhash[i];
        printf("Key: %d\n", b.key);
        printf("Number of directories: %d\n", b.num_directories);
        for (int j = 0; j < b.num_directories; j++)
        {
            printf("Directory %d: %s\n", j + 1, b.directories[j].dirpath.name);
        }
    }
}