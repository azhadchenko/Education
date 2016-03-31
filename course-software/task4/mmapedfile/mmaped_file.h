


struct MmapedFile;


//Compile only with -D_FILE_OFFSET_BITS=64
//Otherwise you will get an error int mopen

struct MmapedFile* mopen(const char* pathname, int flags, int mode);
/*Arguments is same as open();
In case of success, returns pointer to structure. Otherwise reterns 0 and sets errno similar to open.
If returned 0 and errno is 0 - memory allocation failure
*/

ssize_t mread(struct MmapedFile* file, void* buf, size_t count);
/*
Tryes to read count bytes from file into buffer.
In case of success(Were able to read more than 0), return numbers of bytes read. Sets errno similar to mmap in case read < than count.
Otherwise, return -1. Errno is set as mmap failure errno.
*/

ssize_t mwrite(struct MmapedFile* file, void* buf, size_t count);
/*
Tryes to write count bytes from buffer into file.
In case of success(Were able to write more than 0), return numbers of bytes read. Sets errno similar to mmap in case write < than count.
Otherwise, return -1. Errno is set as mmap failure errno.
*/

off_t mseek(struct MmapedFile* file, off_t offset, int whence);
/*
Syntax is similar to fseek. Note - off_t is 64bytes.
Return new offset in file.
If whence or offset incorrect, sets errno to EINVAL. File offset becomes 0
*/


size_t chunk_mseek(struct MmapedFile* file, size_t chunk, int whence);
/*
Similar to mseek, except chunks are manipulated instead of offset.
*/

void chunk_reclaim(struct MmapedFile* file, int count, int chunk);
/*
Reclaims(unmmaps) up to COUNT chunks. This will free up to count*file->chunk_size memory.
Chunk must be in 0..file->max_chunk. Otherwise undefined behaviour.
*/

// HERE COMES THE DANGEROUS ZONE
// IF IT IS POSSIBLE, AVOID MANIPULATING CHUNKS AND MMAPED MOMORY ON YOUR OWN.
// ANY CHUNK AND MEMORY YOU HANDLE CAN BE UNMMAPED AFTER mwrite, mread, chunk_reclaim, mmap_chunk

//Use (struct MmapedFile*)file->chunk_size to know your chunk size.
//If struct, which represent file, is modified, behaviour of any func using this struct is undefined. Expect SIGSEGV

ssize_t calc_chunk(struct MmapedFile* file, off_t offset);
/*
Return number of chunk, which would contain stated offset
If offset is out of file range, do not use it, or you will fall into chaos
*/

int is_mmaped(struct MmapedFile* file, void* ptr, size_t* chunk, size_t* offset_in_chunk);
/*
Returns 1 if ptr is mmaped now. Otherwise 0
If chunk is not null, sets *chunk to number of chunk, which contains ptr.
If offset_in_chunk is no null, sets *offset_in_chunk to ptr offset in chunk.
*/

void* mmap_chunk(struct MmapedFile* file, size_t chunk);
/*
Mmaps requested chunk. Returns ptr to mmaping.
In case of failure, returns -1, sets errno similar as mmap.
*/

void* get_prt(struct MmapedFile* file, size_t chunk);
/*
Returns ptr to chunk. 0 means chunk is unmmaped
*/

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include "mmaped_internals.h"
