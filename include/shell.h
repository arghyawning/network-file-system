#define MAXINPUTLENGTH 1024
#define MAXBUFFER 1024

int invalidPath(char *path);

void custom_read(char *filepath);
void custom_write(char *filepath);
void custom_delete(char *filepath);
void custom_copy(char *srcpath, char *destpath);
void custom_create(char *filepath);
void custom_fileinfo(char *filepath);