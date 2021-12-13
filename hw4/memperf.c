#include "mymalloc.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void validate(){
    srand(0);

    myinit(FIRST_FIT);

    void* ptrs[100];

    int i;
    for (i = 0; i < 100; ++i){
        int size = rand() % 100;
        void* ptr = mymalloc(size);
        assert((size_t)ptr % 8 == 0);
        memset(ptr, -1, size);
        ptrs[i] = ptr;
    }

    for (i = 0; i < 100; ++i){
        myfree(ptrs[i]);
    }

    void* ptr = mymalloc(1000);
    memset(ptr, -1, 1000);
    ptr = myrealloc(ptr, 2000);
    for (i = 0; i < 1000; ++i){
        assert(((char*)ptr)[i] == -1);
    }

    mycleanup();
}

void test(int num_ops,
    int alloc_alg,
    const char* alloc_alg_name,
    unsigned int seed){
    srand(seed); // use the same seed

    struct timeval start;
    struct timeval end;

    void** ptrs = malloc(sizeof(*ptrs) * num_ops);
    memset(ptrs, 0, sizeof(*ptrs) * num_ops);

    gettimeofday(&start, NULL);

    myinit(alloc_alg);

    int i, valid_ops = 0;
    for (i = 0; i < num_ops; ++i){
        int op = rand() % 3;
        if (op == 0){ // malloc
            int size = (rand() % 256) + 1;
            ptrs[i] = mymalloc(size);
            if (ptrs[i]){
                assert((size_t)ptrs[i] % 8 == 0);
                ++valid_ops;
            }
        } else if (op == 1){ // realloc
            if (i > 0){
                int n = 1000;
                int index = rand() % i;
                while (--n && index < i && ptrs[index] == NULL){
                    ++index;
                }
                if (index < i){
                    int size = (rand() % 256) + 1;
                    ptrs[index] = myrealloc(ptrs[index], size);
                    if (ptrs[index]){
                        assert((size_t)ptrs[index] % 8 == 0);
                        ++valid_ops;
                    }
                }
            }
        } else{ // free
            if (i > 0){
                int n = 500;
                int index = rand() % i;
                while (--n && index < i && ptrs[index] == NULL){
                    ++index;
                }
                if (index < i){
                    myfree(ptrs[index]);
                    ptrs[index] = NULL;
                    ++valid_ops;
                }
            }
        }
    }

    double util = utilization();

    mycleanup();

    gettimeofday(&end, NULL);

    double time_used =
        (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    double throughput = valid_ops / time_used;

    // printf("valid ops: %d, time: %lf\n", valid_ops, time_used);
    printf("%s throughput: %g ops/sec\n", alloc_alg_name, throughput);
    printf("%s utilization: %g\n", alloc_alg_name, util);

    free(ptrs);
}

int main(int argc, char** argv){
    validate();

    unsigned int seed = time(NULL);

    // Number of operations
    const int N = 1000000;

    test(N, FIRST_FIT, "First fit", seed);
    test(N, NEXT_FIT, "Next fit", seed);
    test(N, BEST_FIT, "Best fit", seed);

    return 0;
}
