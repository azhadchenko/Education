#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

int main()
{
    log_cond(0 < -3);
    return 0;
}
