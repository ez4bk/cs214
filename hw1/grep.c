#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define STRINGSIZE 255

void decapitalize(char* str){
    for (int i = 0; i <= strlen(str); i++){
        if (str[i] >= 'A' && str[i] <= 'Z'){
            str[i] += 32;
        }
    }
}

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

int main(int argc, char* argv[]){
    if (argc == 1) return 0;
    char* grep = malloc(STRINGSIZE * sizeof(char));
    bool caseInsensitive = false;
    if (argc == 3){
        char* cmd = malloc(STRINGSIZE * sizeof(char));
        strcpy(cmd, argv[1]);
        strcpy(grep, argv[2]);
        if (strcmp(cmd, "-i") == 0){
            caseInsensitive = true;
            decapitalize(grep);
        }
        free(cmd);
    } else if (argc < 3){
        strcpy(grep, argv[1]);
    }

    char* input = malloc(STRINGSIZE * sizeof(char));
    while (fgets(input, STRINGSIZE, stdin) != NULL){
        if (caseInsensitive){
            char* lowerInput = malloc(STRINGSIZE * sizeof(char));
            strcpy(lowerInput, input);
            decapitalize(lowerInput);
            if (grepCheck(grep, lowerInput)) printf("%s", input);
            free(lowerInput);
        }else {
            if (grepCheck(grep, input)) printf("%s", input);
        }

    }
    putchar('\n');

    free(grep);
    free(input);


}