#include <stdio.h>
#include <string.h>

void removeConsecutiveDots(char *path) {
    int len = strlen(path);
    int i, j;

    for (i = 0, j = 0; i < len; i++) {
        if (path[i] == '.' && path[i + 1] == '/' && path[i + 2] == '.') {
            // Skip the consecutive "./"
            i += 1;
        } else {
            // Copy the character to the new string
            path[j++] = path[i];
        }
    }

    // Null-terminate the modified string
    path[j] = '\0';
}

int main() {
    char path[] = "./././././a/b";
    
    printf("Original Path: %s\n", path);

    removeConsecutiveDots(path);

    printf("Modified Path: %s\n", path);

    return 0;
}
