#include "mymalloc.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_SIZE (1 * 1024 * 1024) // 1 MB

#define ALIGNMENT 8

#define HEADER_SIZE 8
#define MIN_BLOCK_SIZE (sizeof(block_t) + HEADER_SIZE)

#define TAG_USED 0x1
#define TAG_PREV_USED 0x2
#define TAG_MASK 0x3

typedef struct block_t{
    unsigned int header;   // contains block size and tags
    unsigned int req_size; // we need this to calculate mem used vs space used
    struct block_t* next;
    struct block_t* prev;
    // size_t footer;
} block_t;

typedef struct{
    int alloc_alg;
    int mem_used;
    void* heap_start;
    void* heap_end;
    block_t* (*alloc_func)(unsigned int size);
    block_t* free_list_head;
    block_t* cur; // for next fit
} heap_t;

heap_t heap;

static unsigned int round_up(unsigned int size){
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

static unsigned int block_size(block_t* blk){
    return blk->header & ~(ALIGNMENT - 1);
}

static unsigned int block_tags(block_t* blk){
    return blk->header & TAG_MASK;
}

static void* payload(block_t* blk){
    return (void*)blk + HEADER_SIZE;
}

static block_t* to_block(void* payload){
    return payload - HEADER_SIZE;
}

static block_t* next_block(block_t* blk){
    void* next = ((void*)blk) + block_size(blk);
    return next < heap.heap_end ? next : NULL;
}

static block_t* prev_block(block_t* blk){
    assert((blk->header & TAG_PREV_USED) == 0);
    return (void*)blk - block_size((block_t*)(((void*)blk) - HEADER_SIZE));
}

static size_t* get_footer(block_t* blk){
    return ((void*)blk) + block_size(blk) - sizeof(size_t);
}

static void set_footer(block_t* blk){
    size_t* footer = get_footer(blk);
    *footer = blk->header;
}

static void set_next_prev_used(block_t* blk){
    block_t* next = next_block(blk);
    if (next){
        next->header |= TAG_PREV_USED;
    }
}

static void clear_next_prev_used(block_t* blk){
    block_t* next = next_block(blk);
    if (next){
        next->header &= ~TAG_PREV_USED;
    }
}

static block_t* first_fit(unsigned int size){
    block_t* blk = heap.free_list_head;
    while (blk != NULL){
        if (block_size(blk) >= size){
            return blk;
        } else{
            blk = blk->next;
        }
    }
    return NULL;
}

static block_t* next_fit(unsigned int size){
    block_t* blk = heap.cur;

    if (blk == NULL){
        blk = heap.free_list_head;
    }

    // Search from cur node to the end
    while (blk != NULL){
        if (block_size(blk) >= size){
            break;
        }
        blk = blk->next;
    }

    // If not found, search from head to cur
    if (blk == NULL && heap.cur != NULL){
        blk = heap.free_list_head;
        while (blk != heap.cur){
            if (block_size(blk) >= size){
                break;
            }
            blk = blk->next;
        }
    }

    if (blk){
        heap.cur = blk->next;
    }

    return (blk && block_size(blk) >= size) ? blk : NULL;
}

static block_t* best_fit(unsigned int size){
    unsigned int best_size = 0;
    block_t* best = NULL;
    block_t* blk = heap.free_list_head;
    while (blk != NULL){
        unsigned int blk_size = block_size(blk);
        if (blk_size == size){
            return blk;
        } else if (blk_size > size){
            if (best == NULL || blk_size < best_size){
                best = blk;
                best_size = blk_size;
            }
        }
        blk = blk->next;
    }
    return best;
}

static void add_free_block(block_t* blk){
    block_t* head = heap.free_list_head;
    blk->next = head;
    if (head != NULL){
        head->prev = blk;
    }
    blk->prev = NULL;
    heap.free_list_head = blk;
}

static void remove_free_block(block_t* blk){
    block_t* next = blk->next;
    block_t* prev = blk->prev;

    if (next != NULL){
        next->prev = prev;
    }

    if (blk == heap.free_list_head){
        assert(prev == NULL);
        heap.free_list_head = next;
    } else{
        assert(prev);
        prev->next = next;
    }

    // Update cur node
    if (blk == heap.cur){
        heap.cur = next;
    }
}

static void coalescing(block_t* blk){
    assert((blk->header & TAG_USED) == 0);

    block_t* prev;
    block_t* next = next_block(blk);

    if ((blk->header & TAG_PREV_USED) == 0){
        prev = prev_block(blk);
        prev->header += block_size(blk);
        set_footer(prev);
        remove_free_block(blk);
        blk = prev;
    }

    if (next && (next->header & TAG_USED) == 0){
        blk->header += block_size(next);
        remove_free_block(next);
        set_footer(blk);
    }
}

// Split this block into 2 blocks
static void split(block_t* blk, unsigned int block_size, unsigned int size){
    blk->header = size | block_tags(blk);

    block_t* next = next_block(blk);
    next->header = (block_size - size) | TAG_PREV_USED;
    set_footer(next);

    add_free_block(next);

    if (blk->header & TAG_USED){
        coalescing(next);
    }
}

void myinit(int allocAlg){
    heap.alloc_alg = allocAlg;
    heap.alloc_func = (allocAlg == FIRST_FIT)
        ? first_fit
        : (allocAlg == NEXT_FIT ? next_fit : best_fit);
    heap.mem_used = 0;

    block_t* blk = malloc(HEAP_SIZE);
    blk->next = NULL;
    blk->prev = NULL;
    blk->header = HEAP_SIZE | TAG_PREV_USED;
    set_footer(blk);

    heap.free_list_head = blk;
    heap.heap_start = blk;
    heap.heap_end = heap.heap_start + HEAP_SIZE;
    heap.cur = blk;
}

void* mymalloc(size_t size){
    if (size == 0){
        return NULL;
    }

    unsigned int req_size = size;

    size += HEADER_SIZE;
    if (size <= MIN_BLOCK_SIZE){
        size = MIN_BLOCK_SIZE;
    } else{
        size = round_up(size);
    }

    // Search in the free list
    block_t* blk = heap.alloc_func(size);
    if (blk == NULL){
        return NULL; // not found
    }

    remove_free_block(blk); // remove from the free list

    unsigned int blk_size = block_size(blk);
    unsigned int remaining_size = blk_size - size;
    if (remaining_size >= MIN_BLOCK_SIZE){
        // Split this block into 2 blocks
        split(blk, blk_size, size);
    } else{
        // Set the following block
        set_next_prev_used(blk);
    }
    blk->header |= TAG_USED; // mark used tag
    blk->req_size = req_size;

    heap.mem_used += req_size;

    return payload(blk);
}

static bool check_block(block_t* blk, unsigned int* size){
    if ((void*)blk < heap.heap_start || (void*)blk >= heap.heap_end){
        fprintf(stderr, "error: not a heap pointer\n");
        return false;
    }

    *size = block_size(blk);

    if (*size < MIN_BLOCK_SIZE || *size > HEAP_SIZE || blk->req_size > *size){
        fprintf(stderr, "error: not a malloced address\n");
        return false;
    }

    if (!(blk->header & TAG_USED)){
        fprintf(stderr, "error: double free\n");
        return false;
    }

    return true;
}

void myfree(void* ptr){
    if (ptr == NULL){
        return;
    }

    // Set the block freed
    block_t* blk = to_block(ptr);
    unsigned int size;

    if (!check_block(blk, &size)){
        return;
    }

    blk->header &= ~TAG_USED;
    set_footer(blk);
    heap.mem_used -= blk->req_size;

    // Set the following block
    clear_next_prev_used(blk);

    // Insert to the free list
    add_free_block(blk);

    coalescing(blk);
}

void* myrealloc(void* ptr, size_t size){
    if (ptr == NULL){
        return mymalloc(size);
    }

    if (size == 0){
        myfree(ptr);
        return NULL;
    }

    // Shrink

    block_t* blk = to_block(ptr);
    unsigned int blk_size;

    if (!check_block(blk, &blk_size)){
        return NULL;
    }

    size_t req_size = size;

    size += HEADER_SIZE;
    if (size <= MIN_BLOCK_SIZE){
        size = MIN_BLOCK_SIZE;
    } else{
        size = round_up(size);
    }

    // Shrink the block if needed
    if (size <= blk_size){
        unsigned int remaining_size = blk_size - size;
        if (remaining_size >= MIN_BLOCK_SIZE){
            // Seperate this block into 2 blocks
            split(blk, blk_size, size);
        }

        heap.mem_used += req_size;
        heap.mem_used -= blk->req_size;

        blk->req_size = req_size;

        return ptr;
    }

    void* new_ptr = mymalloc(req_size);
    if (new_ptr == NULL){
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < blk->req_size; ++i){
        ((unsigned char*)new_ptr)[i] = ((unsigned char*)ptr)[i];
    }

    myfree(ptr);

    return new_ptr;
}

void mycleanup(){
    free(heap.heap_start);
}

double utilization(){
    assert(heap.mem_used >= 0);

    block_t* last = NULL;
    block_t* blk = heap.heap_start;
    while (blk){
        if (blk->header & TAG_USED){
            last = blk;
        }
        blk = next_block(blk);
    }

    if (last == NULL){
        return 1.0;
    }

    double space_used = ((void*)last - heap.heap_start) + block_size(last);

    return heap.mem_used / space_used;
}
