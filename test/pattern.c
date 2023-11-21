// #include <stdio.h>
// #include <string.h>

// void removeConsecutiveDots(char *path) {
//     int len = strlen(path);
//     int i, j;

//     for (i = 0, j = 0; i < len; i++) {
//         if (path[i] == '.' && path[i + 1] == '/' && path[i + 2] == '.') {
//             // Skip the consecutive "./"
//             i += 1;
//         } else {
//             // Copy the character to the new string
//             path[j++] = path[i];
//         }
//     }

//     // Null-terminate the modified string
//     path[j] = '\0';
// }

// int main() {
//     char path[] = "./././././a/b";
    
//     printf("Original Path: %s\n", path);

//     removeConsecutiveDots(path);

//     printf("Modified Path: %s\n", path);

//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Function to extract the substring before the last '/'
// Returns NULL if there is no '/' in the string
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
        return NULL;
    }

    // Copy the substring before the last '/' into the new buffer
    strncpy(substring, path, length);
    substring[length] = '\0';  // Null-terminate the substring

    return substring;
}

int main() {
    const char *path = "./a/b.txt";  // Replace with your path

    char *substring = get_substring_before_last_slash(path);

    if (substring != NULL) {
        printf("Substring before the last '/': %s\n", substring);
        free(substring);  // Free the allocated memory
    } else {
        printf("No '/' in the string.\n");
    }

    return 0;
}


