#ifndef __MY_ALLOC_H__
#define __MY_ALLOC_H__

#include <stddef.h>

typedef enum{ FIRST_FIT, NEXT_FIT, BEST_FIT } ALLOC_ALG;

void myinit(int allocAlg);
void* mymalloc(size_t size);
void myfree(void* ptr);
void* myrealloc(void* ptr, size_t size);
void mycleanup();
double utilization();

#endif // __MY_ALLOC_H__
