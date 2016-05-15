#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


enum State {
    EMPTY = 0,
    READY = 1,
    BUSY = 2
};

struct Chunk {
    void* ptr;
    off_t offset;
    size_t size;
    size_t state;
    size_t refcount;
};

struct Spool {
    unsigned char count;
    struct Chunk data[0];
};

struct Pool {
    struct Spool** data;

    unsigned char spool_size;
    int spool_count;
    int spool_max;
};


#define INIT_SPOOL_COUNT 1

struct Pool* init_pool(){

    size_t spool_size = 64;

    if(sizeof(void*) == 4) {
        spool_size = 128;
    }

    size_t size = spool_size * sizeof(struct Chunk) + sizeof(struct Spool) + sizeof(struct Pool);
    struct Pool* pool = (struct Pool*)calloc(1, size);
    if(!pool)
        return (void*)-1;


    pool -> data = (struct Spool**)calloc(sizeof(void*), INIT_SPOOL_COUNT);
    if(!pool -> data) {
        free(pool);
        return (void*)-1;
    }

    pool -> spool_size = spool_size;
    pool -> data[pool -> spool_count++] = (struct Spool*)((void*)pool + sizeof(struct Pool));
    pool -> spool_max = INIT_SPOOL_COUNT;

    return pool;
}

ssize_t destruct_pool(struct Pool* pool) {
    if(!pool)
        return 0;

    for(int i = 1; i < pool -> spool_count; i++)
        free(pool -> data[i]);

    free(pool);

    return 0;
}

struct Chunk* allocate_chunk(struct Pool* pool){
    if(!pool)
        return 0;

    for(int i = 0; i < pool -> spool_count; i++) {

        if(pool -> data[i] -> count == pool -> spool_size)
            continue;

        struct Chunk* tmp = pool -> data[i] -> data;

        for(int j = 0; j < pool -> spool_size; j++) {
            if(tmp[j].state == EMPTY) {
                tmp[j].state = BUSY;
                tmp[j].refcount++;

                pool -> data[i] -> count++;

                return &tmp[j];
            }

        }
    }

    //actions if all spools are full

    void* free_candidate = 0;

    if(pool -> spool_count == pool -> spool_max) {

        struct Spool** tmp_data = (struct Spool**)calloc(pool->spool_max * 2, sizeof(void*));
        if(!tmp_data)
            return 0;
        memcpy(tmp_data, pool -> data, sizeof(void*) * pool -> spool_count);

        free_candidate = pool -> data;
        pool -> data = tmp_data;
        pool -> spool_max *= 2;
    }

    size_t index = pool -> spool_count;
    struct Spool* tmp = (struct Spool*)calloc(1, sizeof(struct Spool) + sizeof(struct Chunk) * pool -> spool_size);
    if(!tmp)
        return (void*)-1;

    pool -> data[index] = tmp;
    pool -> spool_count++;

    free(free_candidate); //Actually this can be a real problem if someone is handling previous pool -> data

    return allocate_chunk(pool);
}

ssize_t deref_chunk(struct Pool* pool, struct Chunk* item) {
    if(!pool || !item)
        return -1;

    for(int i = 0; i < pool -> spool_count; i++) {
        size_t bottom = (size_t)item - (size_t)(pool -> data[i] -> data);
        size_t top = (size_t)(pool -> data[i] -> data + pool -> spool_size) - (size_t)item;

        if(bottom <= pool -> spool_size * sizeof(struct Chunk) && top <= pool -> spool_size * sizeof(struct Chunk)) {
            item -> refcount--;
            if(!item -> refcount) {
                pool -> data[i] -> count--;
                item -> state = EMPTY;
            }   else {item -> state = READY;}

            return 0;
        }
    }

    return -1;
}


int main(){ //Such test, wow
    struct Pool* pool = init_pool();

    printf("Passed init \n");

    struct Chunk* array[256] = {0};
    for(int i = 0; i < 80; i++) {
        array[i] = allocate_chunk(pool);
        if(array[i] == (void*)-1) {
            printf("WTF \n");
            exit(0);
        }
        array[i] -> state = READY;
    }

    printf("Passed allocation \n");

    for(int i = 0; i < 80; i++) {
        array[i] -> state = BUSY;
        deref_chunk(pool, array[i]);
    }

    printf("Passed dereference \n");

    for(int i = 0; i < 80; i++) {
        array[i] = allocate_chunk(pool);
        array[i] -> state = READY;
    }

    printf("Passed 2nd allocation \n");

    destruct_pool(pool);
}
