#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
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

void ls(){
    DIR* dirp = opendir(".");
    struct dirent* dir;
    int size = 0;

    if (dirp == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    char** filenames = malloc(DEFAULT_SIZE * sizeof(char*));
    for (int i = 0; i < DEFAULT_SIZE; i++){
        filenames[i] = (char*)malloc(DEFAULT_SIZE * sizeof(char));
    }

    while ((dir = readdir(dirp)) != NULL){
        char* filename = malloc(DEFAULT_SIZE * sizeof(char));
        strcpy(filename, dir->d_name);
        if (filename[0] != '.'){
            strcpy(filenames[size], filename);
            size++;
        }
        free(filename);
    }
    closedir(dirp);

    char** new = stringSort(filenames, size);
    for (int i = 0; i < size; i++){
        printf("%s\n", new[i]);
    }

    for (int i = 0; i < DEFAULT_SIZE; i++) free(filenames[i]);
    free(filenames);
    for (int i = 0; i < size; i++) free(new[i]);
    free(new);

}

void lsDashL(){
    DIR* dirp = opendir(".");
    struct dirent* dir;
    struct stat statbuf;
    if (dirp == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((dir = readdir(dirp)) != NULL){
        if ((dir->d_name)[0] == '.') continue;
        if (stat(dir->d_name, &statbuf) < 0) exit(EXIT_FAILURE);
        printf((statbuf.st_mode & S_IRUSR) ? "r" : "-");
        printf((statbuf.st_mode & S_IWUSR) ? "w" : "-");
        printf((statbuf.st_mode & S_IXUSR) ? "x" : "-");
        printf((statbuf.st_mode & S_IRGRP) ? "r" : "-");
        printf((statbuf.st_mode & S_IWGRP) ? "w" : "-");
        printf((statbuf.st_mode & S_IXGRP) ? "x" : "-");
        printf((statbuf.st_mode & S_IROTH) ? "r" : "-");
        printf((statbuf.st_mode & S_IWOTH) ? "w" : "-");
        printf((statbuf.st_mode & S_IXOTH) ? "x" : "-");
        struct passwd *pw = getpwuid(statbuf.st_uid);
        struct group  *gr = getgrgid(statbuf.st_gid);
        if (pw!=NULL){
            printf(" %s", pw->pw_name);
        }else{
            printf(" %d", statbuf.st_uid);
        }
        if (gr!=NULL){
            printf(" %s", gr->gr_name);
        }else {
            printf(" %d", statbuf.st_gid);
        }
        char date[36];
        strftime(date, 36, "%b %d %H:%M", localtime(&(statbuf.st_mtime)));
        printf(" %s", date);
        printf(" %s", dir->d_name);

        printf("\n");
    }

    closedir(dirp);

}

int main(int argc, char* argv[]){
    if (argc == 1) ls();
    if (argc == 2 && strcmp(argv[1], "-l") == 0) lsDashL();
    return 0;
}