#define LOG_LEVEL_1 1

#define log_cond(CONDITION)                                             \
    do {                                                                \
        if(CONDITION)                                                   \
            break;                                                      \
        inner_log(LOG_WARNING, "Condition falied: " #CONDITION "\n");   \
    } while(0)



