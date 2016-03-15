enum LogLevel {
    INFO = 0,
    WARNING,
    ERROR,
    DEBUG
};

#define LOG_LEVEL_3

#define log_cond(CONDITION)
#define log_cond_msg(CONDITION, MSG, ...)

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



