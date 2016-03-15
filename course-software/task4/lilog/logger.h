enum LogLevel {
    INFO = 0,
    WARNING,
    ERROR,
    DEBUG
};

//Some little logger. Thread safe if you define BUFF_COUNT 2-3 times bigger than FLUSH_COUNT
//
//Compile with --gcc=gnu99
//Before start, you must set define LOG_LEVEL_3 LOG_LEVEL_2 LOG_LEVEL_1 LOG_LEVEL_0
//Othserwise none of code included


#define log_cond(CONDITION)
#define log_cond_msg(CONDITION, MSG, ...)
#define _if if

#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2) || defined(LOG_LEVEL_1) || defined(LOG_LEVEL_0)
    #include "log_level_0.h"
#endif

#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2) || defined(LOG_LEVEL_1)
    #undef log_cond
    #include "log_level_1.h"
#endif

#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2)
    #undef log_cond_msg
    #include "log_level_2.h"
#endif

#ifdef LOG_LEVEL_3
    #include "log_level_3.h"
#endif



