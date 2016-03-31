struct MmapedFile {
    int flags;
    int fd;
    off_t max_sz;
    off_t offset;
    size_t chunk_size;
    size_t chunk_max;
    void** associated_memory;
};

#define MB 1024*1024
#define CHUNK_SIZE MB * 16

struct MmapedFile* mopen(const char* pathname, int flags, int mode) {

    struct MmapedFile* finfo = (struct MmapedFile*)calloc(1, sizeof(struct MmapedFile));
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
    finfo -> associated_memory = (void**)calloc(sizeof(void*), finfo -> chunk_max);
    if(!finfo -> associated_memory) {
        free(finfo);
        close(fd);
        return 0;
    }

    finfo -> chunk_size = CHUNK_SIZE;

    return finfo;
}

//Well, here i will use DFC - Discard Furthest Chunk, since it is easy

int get_furthest(struct MmapedFile* file, int chunk) {

    size_t tmp = file -> chunk_max / 2;

    if(tmp >= chunk) {
        for(size_t i = file -> chunk_max - 1; i >= chunk * 2; i--)
            if(file -> associated_memory[i])
                return i;

        for(size_t i = 0; i < chunk / 2; i++) {
            if(file -> associated_memory[i])
                return i;

            if(file -> associated_memory[chunk * 2 - i])
                return i;
        }
    }

    if(tmp < chunk) {
        tmp = chunk - (file -> chunk_max - chunk);

        for(size_t i = 0; i <= chunk - (file -> chunk_max - chunk); i++)
            if(file -> associated_memory[i])
                return i;


        for(size_t i = 0; i < chunk / 2; i++) {
            if(file -> associated_memory[file -> chunk_max - i])
                return i;

            if(file -> associated_memory[tmp + i])
                return i;

        }
    }


    return -1;
}

void chunk_reclaim(struct MmapedFile* file, int count, int chunk) {

    for(int i = count; i > 0; i--) {
        int del = get_furthest(file, chunk);
        if(del == -1 || del == chunk)
            continue;

        int err = munmap(file -> associated_memory[del], file -> chunk_size);
        if(err == -1)
            assert(0); //FIX THIS

        file -> associated_memory[del] = 0;
    }
}

#define MAX_MMAP_TRY 5
#define CHUNK_RECLAIM_ONCE 5

void* inner_mmap(struct MmapedFile* file, int chunk) {

    off_t file_offset = ((off_t)(file -> chunk_size)) * ((off_t)(chunk));

    for(int i = MAX_MMAP_TRY; i > 0; i--) {
        errno = 0;

        file -> associated_memory[chunk] = mmap(NULL, file -> chunk_size, PROT_READ | PROT_WRITE,
            MAP_SHARED, file -> fd, file_offset);

        if((file -> associated_memory[chunk] == (void*)-1) && (errno == ENOMEM))
            chunk_reclaim(file, CHUNK_RECLAIM_ONCE, chunk);
        else
            if(errno == 0)
                return file -> associated_memory[chunk];
    }

    return (void*)-1;
}

ssize_t mread(struct MmapedFile* file, void* buf, size_t count) {
    if(!file)
        return -1;

    if(!(file -> flags & (O_RDONLY | O_RDWR)))
        return -1;


    size_t saved_count = count;

    while(count > 0) {
        off_t offset = file -> offset;
        int chunk = (int)(offset / (off_t)file -> chunk_size);

        void* memptr = file -> associated_memory[chunk];
        if(memptr == 0) {
            memptr = inner_mmap(file, chunk);
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


ssize_t mwrite(struct MmapedFile* file, void* buf, size_t count) {
    if(!file)
        return -1;

    if(!(file -> flags & (O_WRONLY | O_RDWR)))
        return -1;

    size_t saved_count = count;

    while(count > 0) {
        off_t offset = file -> offset;
        int chunk = (int)(offset / (off_t)file -> chunk_size);

        void* memptr = file -> associated_memory[chunk];
        if(memptr == 0) {
            memptr = inner_mmap(file, chunk);
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

off_t mseek(struct MmapedFile* file, off_t offset, int whence) {
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

size_t chunk_mseek(struct MmapedFile* file, size_t chunk, int whence) {
    if(!file)
        return -1;

    switch (whence) {
        case SEEK_SET :
            if((file -> offset = (off_t)file -> chunk_size * (off_t)chunk) >= file -> max_sz) {
                file -> offset = 0;
                errno = EINVAL;
                return (off_t)-1;
            }
            break;

        case SEEK_CUR :
            if((file -> offset = file -> offset + chunk * file -> chunk_size) >= file -> max_sz) {
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

int is_mmaped(struct MmapedFile* file, void* ptr, size_t* chunk, size_t* offset_in_chunk) {

    void* tmp = 0;

    for(size_t i = 0; i < file -> chunk_max; i++)
        if(tmp = file -> associated_memory[i])
            if(ptr >= tmp && ptr < tmp + file -> chunk_size) {
                if(chunk)
                    *chunk = i;
                if(offset_in_chunk)
                    *offset_in_chunk = tmp + file -> chunk_size - ptr;

                return 1;
            }

    return 0;
}

void* mmap_chunk(struct MmapedFile* file, size_t chunk) {
    if(!file)
        return (void*)-1;

    if(file -> chunk_max <= chunk)
        return (void*)-1;

    if(file -> associated_memory[chunk])
        return 0;

    return inner_mmap(file, chunk);
}

ssize_t calc_chunk(struct MmapedFile* file, off_t offset) {
    if(!file)
        return -1;

    return (ssize_t)(offset / file -> chunk_size);
}

void* get_prt(struct MmapedFile* file, size_t chunk) {
    if(!file)
        return (void*)-1;

    return file -> associated_memory[chunk];
}
