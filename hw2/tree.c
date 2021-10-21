#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define DEFAULT_SIZE 256

int strcmpi(char* a, char* b){
    int i;
    for (i = 0; a[i] && b[i]; ++i){
        if (a[i] == b[i] || (a[i] ^ 32) == b[i])
            continue;
        else
            break;
    }
    if (a[i] == b[i]) return 0;
    if ((a[i] | 32) < (b[i] | 32)) return -1;
    return 1;
}

int string_cmp(const void* a, const void* b){

    return strcmpi(*(char**)a, *(char**)b);

}

char** stringSort(char** filenames, int size){
    if (filenames == NULL) return filenames;

    char** new = malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++){
        new[i] = (char*)malloc(DEFAULT_SIZE * sizeof(char));
        strcpy(new[i], filenames[i]);
    }

    qsort(new, size, sizeof(char*), string_cmp);

    return new;

}

void printdir(char* dir, int depth){
    int size = 0;
    DIR* dp;
    struct dirent* entry;
    struct stat statbuf;
    if ((dp = opendir(dir)) == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    chdir(dir);

    char** filenames = malloc(DEFAULT_SIZE * sizeof(char*));
    for (int i = 0; i < DEFAULT_SIZE; i++){
        filenames[i] = (char*)malloc(DEFAULT_SIZE * sizeof(char));
    }

    while ((entry = readdir(dp)) != NULL){
        if ((entry->d_name)[0] == '.') continue;
        stat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)){
            printf("%*s- %s\n", depth, "", entry->d_name);
            printdir(entry->d_name, depth + 2);
        } else{
            //printf("%*s- %s\n", depth, "", entry->d_name);
            strcpy(filenames[size], entry->d_name);
            size++;
        }
    }

    char** new = stringSort(filenames, size);
    for (int i = 0; i < size; i++){
        printf("%*s- %s\n", depth, "", new[i]);
    }
    for (int i = 0; i < size; i++) free(new[i]);
    free(new);
    for (int i = 0; i < DEFAULT_SIZE; i++) free(filenames[i]);
    free(filenames);
    chdir("..");
    closedir(dp);
}

int main(int argc, char* argv[]){
    printf(".\n");
    printdir(".", 0);

    return 0;
}