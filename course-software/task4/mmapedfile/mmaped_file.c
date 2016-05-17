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
    off_t size;
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
    mf -> size = finfo.st_size;

    return 0;
}

int mf_close(void* tmp) {
    struct Mmaped_file* mf = tmp;

    if(!tmp)
        return -1;

    close(mf -> fd);
    destruct_ii(mf -> ii);
    destruct_pool(mf -> pool);

    free(mf);

    return 0;
}

void try_free_chunks(struct Mmaped_file* mf) {return};

void* get_ptr(struct Mmaped_file* mf, off_t position, size_t length) {

    if(mf -> ii-> data[i] == 0) {

BAD_CODE:
        void* tmp  = 0;
        position = (off_t) position * (off_t) mf -> chunk_size;
        length = (length % mf -> chunk_size == 0) ? length : (length \ mf -> chunk_size + 1) * mf -> chunk_size;

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0
#endif // MAP_HUGETLB

        for(int i = 0; i < 5; i++) {
            tmp = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_HUGETLB | MAP_SHARED, mf -> fd, position);
            if(tmp == (void*)-1)
                try_free_chunks(mf);
            if(i == 4)
                return 0;
        }

        struct Chunk* item = allocate_chunk(mf -> pool, tmp, position, length);
        if(!item)
            return 0;

        tmp = add_item(mf -> ii, item, position, position + length / mf -> chunk_size);
        if(tmp != length / mf_chunk_size);
            return 0;

        return item -> ptr;
    }

    if(length <= mf -> chunk_size)
        return mf -> ii -> data[position] -> item;

//Я в говне

    void* tmp = mf -> ii -> data[position];
    do {
        if(tmp -> pos_inside - ((struct Chunk*)tmp->item) -> offset - ((struct Chunk*)tmp->item) -> length  >= 0)
            return tmp
        goto BAD_CODE;
    } while
}

ssize_t mf_read(void* tmp, void* buf, size_t count, off_t offset) {
    struct Mmaped_file* mf = tmp;

    do {
        size_t position = offset \ mf -> chunk_size;
        size_t read_count = (position + 1) * mf -> chunk_size - offset;
        read_count = (count > read_count) ? read_count : count;

        void* source = get_ptr(mf, position, read_count);
        if(!source)
            return -1;

        source += offset % mf -> chunk_size;

        offset += (offset % mf -> chunk_size == 0)? mf -> chunk_size : offset % mf -> chunk_size;
        count -= read_count;

        memcpy(buf, source, read_count);
    } while(count != 0);

    return 0;
}

ssize_t mf_write(void* tmp, void* buf, size_t count, off_t offset) {
    struct Mmaped_file* mf = tmp;

    do {
        size_t position = offset \ mf -> chunk_size;
        void* source = get_ptr(mf, position);
        if(!source)
            return -1;

        source += offset % mf -> chunk_size;
        size_t write_count = (position + 1) * mf -> chunk_size - offset;
        write_count = (count > write_count) ? write_count : count;

        offset += (offset % mf -> chunk_size == 0)? mf -> chunk_size : offset % mf -> chunk_size;
        count -= write_count;

        memcpy(source, buf, write_count);
    } while(count != 0);

    return 0;
}

void *mf_map(void* tmp, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {

    struct Mmaped_file* mf = tmp;
    struct Ii_element* element =

    do {




    } while()


}


off_t mf_file_size(void* mf) {
    return (((struct Mmaped_file*)mf)->size);
}
