#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#define ARRAYSIZE 255

int strcmpi(char* a, char* b){
    int i;
    for (i = 0; a[i] && b[i]; ++i)    {
        if (a[i] == b[i] || (a[i] ^ 32) == b[i])
            continue;
        else
            break;
    }

    if (a[i] == b[i]) return 0;

    if ((a[i] | 32) < (b[i] | 32)) return -1;

    return 1;
}

int cmpfunc(const void* a, const void* b){

    return strcmpi(*(char**)a, *(char**)b);
}

void insertionSort(int arr[], int n){
    int i, key, j;
    for (i = 1; i < n; i++){
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > key){
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

void numberSort(){
    char* input = malloc(ARRAYSIZE * sizeof(char));
    int* numbers = malloc(sizeof(int));
    int i = 0;
    while (fgets(input, ARRAYSIZE, stdin) != NULL){
        numbers[i] = atoi(input);
        i++;
        numbers = realloc(numbers, (i + 1) * sizeof(int));
    }
    
    insertionSort(numbers, i);
    putchar('\n');

    for (int j = 0; j < i; j++)
        printf("%d\n", numbers[j]);

    free(input);
    free(numbers);

}

void stringSort(){
    char input[ARRAYSIZE];
    char** strings = NULL;
    int i = 0;

    while (fgets(input, ARRAYSIZE, stdin) != NULL){
        input[strcspn(input, "\n")] = 0;
        strings = (char**)realloc(strings, (i + 1) * sizeof(char*));
        strings[i] = malloc(strlen(input) + 1);
        strcpy(strings[i], input);
        i++;
    }
    if (strings == NULL) return;
    
    qsort(strings, i, sizeof(char*), cmpfunc);

    for (int j = 0; j < i; j++){
        printf("%s\n", strings[j]);
        free(strings[j]);
    }

    free(strings);
}

int main(int argc, char* argv[]){
    bool numberMode = false;
    if (argc == 2){
        char* cmd = malloc(ARRAYSIZE * sizeof(char));
        if (strcmp(cmd, "-n")) numberMode = true;
        free(cmd);
    }

    if (numberMode){
        numberSort();
    } else{
        stringSort();
    }

}