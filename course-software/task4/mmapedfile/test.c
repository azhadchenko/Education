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


int main(int argc, char** argv) {

    void* lol = mf_open("testfile.c");
    mf_close(lol);

    return 0;
}
