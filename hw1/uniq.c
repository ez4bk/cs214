#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define ARRAYSIZE 255

int main(int argc, char* argv[]){
    char input[ARRAYSIZE];
    char** strings = NULL;
    int n = 0;

    while (fgets(input, ARRAYSIZE, stdin) != NULL){
        input[strcspn(input, "\n")] = 0;
        strings = (char**)realloc(strings, (n + 1) * sizeof(char*));
        strings[n] = malloc(strlen(input) + 1);
        strcpy(strings[n], input);
        n++;
    }
    if (strings == NULL) return 0;

    putchar('\n');

    int count = 1;
    for (int i = 1; i < ARRAYSIZE; i++){
        if (strcmp(strings[i - 1], strings[i]) == 0){
            count += 1;
            if (i == n - 1){
                printf("%d %s\n", count, strings[i - 1]);
                return 0;
            }
        } else{
            printf("%d %s\n", count, strings[i - 1]);
            count = 1;
        }

    }

    for (int i = 0; i < n; i++){
        printf("%s\n", strings[i]);
        free(strings[i]);
    }
    free(strings);

}