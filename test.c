#include <stdio.h>
#include <dirent.h>

int main() {
    const char *dirPath = "public/html";
    DIR *dirStream = opendir(dirPath);

    if (dirStream != NULL) {
        struct dirent *dirEntry;

        // Read and print each entry in the directory
        while ((dirEntry = readdir(dirStream)) != NULL) {
            printf("%s\n", dirEntry->d_name);
        }

        closedir(dirStream);
    } else {
        perror("opendir");
        return 1;
    }

    return 0;
}
