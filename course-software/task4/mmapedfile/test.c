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

#include "mmaped_file.h"

#define SZ 1024*1024*16

int main(int argc, char** argv) {

    printf("%d %d \n", sizeof(off_t), sizeof(void*));
    fflush(stdout);

    char* buff = (char*)calloc(1, SZ);
    buff[SZ - 2] = 'A';
    buff[SZ - 3] = 'B';

    struct MmapedFile* finfo = mopen(argv[1], O_RDWR, 0);

// smallfile test
/*    mread(finfo, buff, SZ);
//    mwrite(finfo, buff, SZ);
*/

// bigfile test

    for(int i = 0; i < 256; i++)
        mwrite(finfo, buff, SZ);

    return 0;
}
