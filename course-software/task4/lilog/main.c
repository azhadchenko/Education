#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <execinfo.h>

#define LOG_LEVEL_3

#include "logger.h"


void call3() {
    _if(0)
        return;
};

void call2() {
    call3();
};

void call1() {
    call2();
};

void call0() {
    call1();
};


int main(int argc, char** argv)
{
    init_logger(argv, NULL);

    lilog(INFO, "Such log");
    lilog(DEBUG, "So useful");
    lilog(INFO, "SUCH DATA %d", getpid());
    lilog_cond(0 < -3);
    lilog_cond_msg(!!0, "wow %s", "WOW");


    lilog_print_stack();

    call0();

    lilog_finish(argv);


    return 0;
}

