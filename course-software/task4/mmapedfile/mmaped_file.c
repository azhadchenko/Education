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

#include "chunk_manager.h"
#include "inverted_index.h"

struct Mmaped_file{
    size_t status;
    struct Pool* pool;
    struct Inverted_index* ii;
    size_t chunk_size;
    size_t chunk_count;
    int fd;

};

#define DEFAULT_CHUNK_SIZE 1024*1024*16

void* mf_open(const char* pathname) {
    if(pathname)
        return 0;

    int fd = open(pathname, O_RDWR);
    if(fd == -1)
        return 0;

    struct Mmaped_file* mf = (struct Mmaped_file*)calloc(1, sizeof(struct Mmaped_file));
    if(!mf) {
        close(fd);
        return 0;
    }

    mf -> fd = fd;

    mf -> pool = init_pool();
    if(!mf -> pool) {
        close(fd);
        free(mf);
        return 0;
    }

    struct stat finfo = {0};
    int err = fstat(fd, &finfo);
    if(err == -1)
        return 0;

    size_t chunk_size = DEFAULT_CHUNK_SIZE;
    size_t chunk_count = finfo.st_size / chunk_size;
    while(chunk_count > 1000) {
        chunk_size *= 2;
        chunk_count = finfo.st_size / chunk_size;
    }

    mf -> ii = init_ii(chunk_count);
    if(!mf -> ii) {
        close(fd);
        destruct_pool(mf -> pool);
        free(mf);
    }

    mf -> chunk_size = chunk_size;
    mf -> chunk_count = chunk_count;

    return 0;
}
