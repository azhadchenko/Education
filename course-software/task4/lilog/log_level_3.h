#define BACKTRACE_MAX 16

#ifndef BACKTRACE_ENABLE
#define BACKTRACE_ENABLE 1
#endif // BACKTRACE_ENABLE

void lilog_print_stack() {
    void* buffer[BACKTRACE_MAX] = {0};
    char** strings = 0;
    int left_len = MAX_BUFF;
    size_t offset = 0;

    int nptrs = backtrace(buffer, BACKTRACE_MAX);

    strings = backtrace_symbols(buffer, nptrs);
    if(!strings)
        lilog(DEBUG, "Unable to get backtrace");

    int index = get_index();
    char* tmp = prepare_log(DEBUG, index, &left_len);

    for(int i = 0; i < nptrs; i++)
        if(left_len - offset > 0)
            offset += snprintf(tmp + offset, left_len - offset, "%s\n", strings[i]);


    append_log(index);

    free(strings);
}

#define _if(CONDITION)              \
    if(!CONDITION) {                \
        lilog_cond(CONDITION);      \
        if(BACKTRACE_ENABLE)        \
            lilog_print_stack();    \
    } else if(CONDITION)


