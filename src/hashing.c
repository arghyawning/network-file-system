#include "../include/common.h"

void initialize_hash_table(bucket *fileshash)
{
    for (int i = 0; i < MAX_PATH_SIZE + 1; i++)
    {
        fileshash[i].key = i;
        fileshash[i].num_files = 0;
    }
}

void add_file_in_hash(char *filename, int ssid, bucket *fileshash)
{

    char *filename_updated = strrchr(filename, '/') + 1;

    int key = strlen(filename_updated);
    if (fileshash[key].num_files == MAX_FILES)
    {
        printf("[-]Hash table bucket full. Cannot store more files.\n");
        return;
    }
    ff temp;
    temp.ssid = ssid;
    strcpy(temp.filepath.name, filename);
    fileshash[key].files[fileshash[key].num_files] = temp;
    printf("before: number of files %d for key %d\n", fileshash[key].num_files, key);
    fileshash[key].num_files++;
    printf("after: number of files %d for key %d\n", fileshash[key].num_files, key);
}

void store_in_hash(struct CombinedFilesInfo *files, bucket *fileshash)
{
    for (int i = 0; i < files->numberOfFiles; i++)
    {
        char *filename = strrchr(files->files[i].name, '/') + 1;

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

ff fileSearchWithHash(char *searchfilename, bucket *fileshash)
{
    int key = strlen(searchfilename);
    bucket b = fileshash[key];

    printf("Number of files with key %d: %d\n", key, b.num_files);

    for (int i = 0; i < b.num_files; i++)
    {
        // retrieving file name from path
        printf("Before: File %d: %s\n", i + 1, b.files[i].filepath.name);
        char *filename = strrchr(b.files[i].filepath.name, '/') + 1;
        printf("After: File %d: %s\n", i + 1, filename);
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

void removeHashEntry(char *filename, bucket *fileshash)
{
    int key = strlen(filename);
    bucket *b = &fileshash[key];

    printf("Number of files with key %d: %d\n", key, b->num_files);

    for (int i = 0; i < b->num_files; i++)
    {
        // retrieving file name from path
        printf("Before: File %d: %s\n", i + 1, b->files[i].filepath.name);
        char *filename_updated = strrchr(b->files[i].filepath.name, '/') + 1;
        printf("After: File %d: %s\n", i + 1, filename_updated);
        if (filename_updated == NULL)
        {
            strcpy(filename, b->files[i].filepath.name);
        }

        // comparing file name with searchfilename
        if (strcmp(filename_updated, filename) == 0)
        {
            // Shift elements to the left to fill the gap
            printf("deleting %s\n", filename);

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

void print_hash_table(bucket *fileshash)
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
