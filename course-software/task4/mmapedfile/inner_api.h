#include "mapped_file.h"


struct MmapedFile;


//Compile only with -D_FILE_OFFSET_BITS=64
//Otherwise you will get an error int mopen

mf_handle_t mf_open(const char* pathname, size_t max_memory_usage){
    struct _MmapedFile* res = _mopen(pathname, O_RDWR, 0);
    return res;
}

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf) {

    off_t err = _mseek(mf, offset, SEEK_SET);
    if(err == (off_t)-1)
        return -1;

    return _mread(mf, buf, size);
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf) {

    off_t err = _mseek(mf, offset, SEEK_SET);
    if(err == (off_t)-1)
        return -1;

    return _mwrite(mf, (void*)buf, size);
}

int mf_close(mf_handle_t mf) {
    return _finish(mf);
}

ssize_t mf_file_size(mf_handle_t mf) {
    return _mseek(mf, 0, SEEK_END);
}

mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size){
    return _mf_alloc(mf, offset, &size);
}

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
