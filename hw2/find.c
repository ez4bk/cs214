#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#define DEFAULT_SIZE 256

bool grepCheck(char* key, char* input){
    int keylen = strlen(key);
    int inputlen = strlen(input);

    for (int i=0; i<=(inputlen-keylen); i++){
        int j;
        for (j=0; j<keylen; j++){
            if (input[i+j] != key[j]) break;
        }
        if (j == keylen) return true;
    }
    return false;
}

void printPath(char** path, int* index){

    for (int i=0; i<*index+1; i++){
        printf("%s", path[i]);
    }
    printf("\n");
}

void printdir(char* dir, char** path, int* index, char* grep){
    DIR* dp;
    struct dirent* entry;
    struct stat statbuf;
    if ((dp = opendir(dir)) == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    chdir(dir);
    while ((entry = readdir(dp)) != NULL){
        if ((entry->d_name)[0] == '.') continue;
        stat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)){
            char currPath[DEFAULT_SIZE];
            strcpy(currPath, entry->d_name);
            strcat(currPath, "/");
            strcpy(path[*index], currPath);
            *index += 1;
            printdir(entry->d_name, path, index, grep);
            *index -= 1;
        }else if (grepCheck(grep, entry->d_name)){
            strcpy(path[*index], entry->d_name);
            printPath(path, index);
            
        }
        
    }

    chdir("..");
    closedir(dp);
}

int main(int argc, char* argv[]){
    char** path = malloc(DEFAULT_SIZE * sizeof(char*));
    for (int i = 0; i < DEFAULT_SIZE; i++){
        path[i] = (char*)malloc(DEFAULT_SIZE * sizeof(char));
    }

    strcpy(path[0], "./");
    int size = 1;
    if (argc == 2){
        printdir(".", path, &size, argv[1]);
    }else return 0;

    for (int i = 0; i < DEFAULT_SIZE; i++) free(path[i]);
    free(path);

    return 0;
}