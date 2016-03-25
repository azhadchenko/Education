#define lilog_cond(CONDITION) lilog_cond_inner(CONDITION, WARNING)

#define lilog_cond_inner(CONDITION, LEVEL)                              \
    do {                                                                \
        if(CONDITION)                                                   \
            break;                                                      \
        lilog(LEVEL, "Condition falied: " #CONDITION);                  \
    } while(0)



