#define MAX_BUFF 1024

#define log_cond_msg(CONDITION, MSG, ...)                       \
    do {                                                        \
        char BUFF[MAX_BUFF] = {0};                              \
        if(CONDITION)                                           \
            break;                                              \
                                                                \
        snprintf(BUFF, MAX_BUFF, "Condition failed: "           \
            #CONDITION " with msg: " MSG "\n", __VA_ARGS__);    \
                                                                \
        inner_log(LOG_ERROR, BUFF);                             \
    } while(0)

