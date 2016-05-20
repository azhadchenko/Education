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

#include "mapped_file.h"
#include "chunk_manager.h"
#include "inverted_index.h"


void ii_unit_test(){
    struct Inverted_index* ii = init_ii(256);

    printf("II:Init complete\n");
    fsync(STDOUT_FILENO);

    for(size_t i = 0; i < 128; i++) {
        if(add_item(ii, (struct Ii_element*)i, i, i + 5) != 0)
            assert(5);
    }
    printf("II:Add complete\n");
    fsync(STDOUT_FILENO);


    for(size_t i = 0; i < 100; i++) {
        if(delete_item(ii, (struct Ii_element*)i, i, i) != 0)
            assert(0);
    }

    printf("II:Delete complete\n");
    fsync(STDOUT_FILENO);


    for(size_t i = 0; i < 128; i++) {
        if(add_item(ii, (struct Ii_element*)(i + 128), i + 128, i + 128) != 0)
            assert(0);
    }

    printf("II:Add2 complete\n");
    fsync(STDOUT_FILENO);


    assert(!destruct_ii(ii));
}


void cm_unit_test() {
    struct Pool* pool = init_pool();

    printf("CM:Passed init \n");

    struct Chunk* array[256] = {0};
    for(size_t i = 0; i < 80; i++) {
        array[i] = allocate_chunk(pool, (void*)i, i, i);
        if(array[i] == (void*)-1) {
            printf("WTF \n");
            exit(0);
        }
    }

    printf("CM:Passed allocation \n");

    for(size_t i = 0; i < 80; i++) {
        deref_chunk(pool, array[i]);
    }

    printf("CM:Passed dereference \n");

    for(size_t i = 0; i < 80; i++) {
        array[i] = allocate_chunk(pool, (void*)(i + 80), i + 80, i+ 80);
    }

    printf("CM:Passed 2nd allocation \n");

    destruct_pool(pool);
}


int main(int argc, char** argv) {

    void* lol = mf_open("testfile.c");
    mf_close(lol);

    ii_unit_test();
    cm_unit_test();

    return 0;
}
