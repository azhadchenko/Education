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

struct _associated_mem {
    void* ptr;
    void* next;
    unsigned int ref_count;
};

struct _MmapedFile {
    int flags;
    int fd;
    off_t max_sz;
    off_t offset;
    size_t chunk_size;
    size_t chunk_max;
    struct _associated_mem* data;
};

#define MB 1024*1024
#define CHUNK_SIZE MB * 16

struct _MmapedFile* _mopen(const char* pathname, int flags, int mode) {

    struct _MmapedFile* finfo = (struct _MmapedFile*)calloc(1, sizeof(struct _MmapedFile));
    if(!finfo)
        return 0;

    finfo -> flags = flags;

    int fd = open(pathname, flags, mode);
    if(fd == -1) {
        free(finfo);
        return 0;
    }
    finfo -> fd = fd;

    struct stat fs = {0};
    int err = fstat(fd, &fs);
    if(err == -1) {
        free(finfo);
        close(fd);
        return 0;
    }

    finfo -> max_sz = fs.st_size;
    finfo -> chunk_max =  (size_t)(finfo -> max_sz / ((off_t)(CHUNK_SIZE))) + 1;
    finfo -> data = (struct _associated_mem*)calloc(sizeof(struct _associated_mem), finfo -> chunk_max);
    if(!finfo -> data) {
        free(finfo);
        close(fd);
        return 0;
    }

    finfo -> chunk_size = CHUNK_SIZE;

    return finfo;
}

//Well, here i will use DFC - Discard Furthest Chunk, since it is easy

size_t _get_furthest(struct _MmapedFile* file, size_t chunk) {

    size_t tmp = file -> chunk_max / 2;

    if(tmp >= chunk) {
        for(size_t i = file -> chunk_max - 1; i >= chunk * 2; i--)
            if(file -> data[i].ptr && !(file -> data[i].ref_count))
                return i;

        for(size_t i = 0; i < chunk / 2; i++) {
            if(file -> data[i].ptr && !(file -> data[i].ref_count))
                return i;

            if(file -> data[chunk * 2 - i].ptr && !(file -> data[chunk * 2 - i].ref_count))
                return i;
        }
    }

    if(tmp < chunk) {
        tmp = chunk - (file -> chunk_max - chunk);

        for(size_t i = 0; i <= chunk - (file -> chunk_max - chunk); i++)
            if(file -> data[i].ptr && !(file -> data[i].ref_count))
                return i;


        for(size_t i = 0; i < chunk / 2; i++) {
            if(file -> data[file -> chunk_max - i].ptr && !(file -> data[file -> chunk_max - i].ref_count))
                return i;

            if(file -> data[tmp + i].ptr && !(file -> data[file -> chunk_max - i].ref_count))
                return i;
        }
    }


    return -1;
}

void _chunk_reclaim(struct _MmapedFile* file, int count, size_t chunk) {

    for(int i = count; i > 0; i--) {
        size_t del = _get_furthest(file, chunk);
        if(del == -1 || del == chunk)
            continue;

        int err = munmap(file -> data[del].ptr, file -> chunk_size);
        if(err == -1)
            assert(0); //FIX THIS

        file -> data[del].ptr = 0;
    }
}

#define MAX_MMAP_TRY 5
#define CHUNK_RECLAIM_ONCE 5

void* _inner_mmap(struct _MmapedFile* file, size_t chunk, void* wishaddr) {

    off_t file_offset = ((off_t)(file -> chunk_size)) * ((off_t)(chunk));

    for(int i = MAX_MMAP_TRY; i > 0; i--) {
        errno = 0;

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0
#endif // MAP_HUGETLB


        file -> data[chunk].ptr = mmap(wishaddr, file -> chunk_size, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_HUGETLB, file -> fd, file_offset);


        if((file -> data[chunk].ptr == (void*)-1) && (errno == ENOMEM)) {
            file -> data[chunk].ptr = 0;
            _chunk_reclaim(file, CHUNK_RECLAIM_ONCE, chunk);
        }
        else {
            if(file -> data[chunk].ptr != wishaddr) {
                munmap(file -> data[chunk].ptr, file -> chunk_size);
                return (void*)-1;
            }

            if(errno == 0)
                return file -> data[chunk].ptr;
        }
    }

    return (void*)-1;
}

ssize_t _mread(struct _MmapedFile* file, void* buf, size_t count) {
    if(!file)
        return -1;

    if(!(file -> flags & (O_RDONLY | O_RDWR)))
        return -1;


    size_t saved_count = count;

    while(count > 0) {
        off_t offset = file -> offset;
        size_t chunk = (size_t)(offset / (off_t)file -> chunk_size);

        void* memptr = file -> data[chunk].ptr;
        if(memptr == 0) {
            memptr = _inner_mmap(file, chunk, 0);
            if(memptr == (void*)-1)
                break;
        }

        memptr += (size_t)(file -> offset % (off_t)file -> chunk_size);
        size_t len = memptr + file -> chunk_size - memptr;

        if(count < len) {
            len = count;
            count = 0;
        } else
            count -= len;

        memcpy(buf, memptr, len);
        buf += len;
        file -> offset += len;
    }

    return saved_count - count;
}


ssize_t _mwrite(struct _MmapedFile* file, void* buf, size_t count) {
    if(!file)
        return -1;

    if(!(file -> flags & (O_WRONLY | O_RDWR)))
        return -1;

    size_t saved_count = count;

    while(count > 0) {
        off_t offset = file -> offset;
        size_t chunk = (size_t)(offset / (off_t)file -> chunk_size);

        void* memptr = file -> data[chunk].ptr;
        if(memptr == 0) {
            memptr = _inner_mmap(file, chunk, 0);
            if(memptr == (void*)-1)
                break;
        }

        memptr += (size_t)(file -> offset % (off_t)file -> chunk_size);
        size_t len = memptr + file -> chunk_size - memptr;

        if(count < len) {
            len = count;
            count = 0;
        } else
            count -= len;

        memcpy(memptr, buf, len);
        buf += len;
        file -> offset += len;
    }

    return saved_count - count;
}

off_t _mseek(struct _MmapedFile* file, off_t offset, int whence) {
    if(!file)
        return -1;

    switch (whence) {
        case SEEK_SET :
            if((file -> offset = offset) >= file -> max_sz) {
                file -> offset = 0;
                errno = EINVAL;
                return (off_t)-1;
            }
            break;

        case SEEK_CUR :
            if((file -> offset = file -> offset + offset) >= file -> max_sz) {
                file -> offset = 0;
                errno = EINVAL;
                return (off_t)-1;
            }
            break;

        case SEEK_END : file -> offset = file -> max_sz;
            break;

        default       : errno = EINVAL;
                        return (off_t)-1;
    }

    return file -> offset;
}



int _finish(struct _MmapedFile* file){

    if(!file)
        return -1;

    free(file->data);
    close(file->fd);
    free(file);

    return 0;
}

void* _mf_alloc(struct _MmapedFile* file, off_t offset, size_t* size) {
    if(*size == 0)
        return 0;

    if(!file)
        return (void*)-1;

    size_t chunk = (size_t)(offset / (off_t)file -> chunk_size);
    size_t until = *size / file -> chunk_size + 1;

    void* result = 0;
    if(!file -> data[chunk].ptr) {
        result = _inner_mmap(file, chunk, 0);
        if(result == (void*)-1)
            return result;
    }

    result += (size_t)(offset % (off_t)file -> chunk_size);
    void* next_ptr = result + file -> chunk_size;

    for(chunk++ ; chunk <= until; chunk++) {
        if(!file -> data[chunk].ptr) {
            if(file -> data[chunk].ptr == next_ptr) {
                next_ptr += file -> chunk_size;
                continue;
            }
            *size = next_ptr - result;
            errno = EINVAL;
            return (void*)-1;
        }

        void* res = _inner_mmap(file, chunk, next_ptr);
        if(res == (void*)-1) {
            errno = EINVAL;
            return res;
        }
        next_ptr += file -> chunk_size;
    }

    return 0;
}



#include "inner_api.h"
int main(){}

////Completely new///


