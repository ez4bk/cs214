#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

bool is_prime(int n){
    if (n < 2){
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++){
        if ((n % i) == 0){
            return false;
        }
    }
    return true;
}

void factorize(int num){
    if (is_prime(num)){
        printf(" %d", num);
        return;
    }

    for (int i = 2; i <= num; i++){
        if (is_prime(i) && num % i == 0){
            printf(" %d", i);
            return factorize(num / i);
        }
    }
    return;
}

int main(int argc, char* argv[]){
    if (argc == 1) return 0;
    int num = atoi(argv[1]);

    if (is_prime(num)){
        printf("%d", num);
        return 0;
    }

    for (int i = 2; i <= num; i++){
        if (is_prime(i) && num % i == 0){
            printf("%d", i);
            num = num / i;
            break;
        }
    }
    factorize(num);
    return 0;
}