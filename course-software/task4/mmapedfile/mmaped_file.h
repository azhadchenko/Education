


struct MmapedFile {

};

struct MmapedFile* mopen(const char* pathname, int flags, int mode);
ssize_t mread(struct MmapedFile* file, const void* buf, size_t count);
ssize_t mwrite(struct MmapedFile* file, const void* buf, size_t count);
ssize_t mseek(struct MmapedFile* file, size_t offset, int whence);
