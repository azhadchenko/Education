#ifndef MAX_BUFF
#define MAX_BUFF 512
#endif // MAX_BUFF

#ifndef BUFF_COUNT
#define BUFF_COUNT 32
#endif // BUFF_COUNT

#ifndef FLUSH_COUNT
#define FLUSH_COUNT 24
#endif // FLUSH_COUNT

#define LOG_LEVEL 0

struct Logger {
    char* ring;
    unsigned int index;
    unsigned int last_printed;
    int output_fd;
    int flush_busy;
} logger = {0};


unsigned int get_index() {
    int old_index = 0, index = 0;
    do {
        old_index = logger.index;
        index = (old_index + 1) % BUFF_COUNT;
    } while(!__sync_bool_compare_and_swap(&(logger.index), old_index, index));

    return old_index;
}

#define KNRM  "\x1B[0m["
#define KRED  "\x1B[31m["
#define KGRN  "\x1B[32m["
#define KYEL  "\x1B[33m["
#define KBLU  "\x1B[34m["
#define KMAG  "\x1B[35m["
#define KCYN  "\x1B[36m["
#define KWHT  "\x1B[37m["
#define RESET "] \033[0m"

#define GET_MODE_STR(MODE) #MODE

char* get_mode(int mode) {
    switch(mode) {
    case INFO :
        return KWHT GET_MODE_STR(INFO) RESET;
    case WARNING :
        return KBLU GET_MODE_STR(WARNING) RESET;
    case ERROR :
        return KYEL GET_MODE_STR(ERROR) RESET;
    case DEBUG :
        return KRED GET_MODE_STR(DEBUG) RESET;
    default :
        return KMAG GET_MODE_STR(UNKNOWN) RESET;
    }
}

char* prepare_log(int message_level, unsigned int index, int* left_len) {
    size_t offset = index * MAX_BUFF;
    time_t t = {0};
    unsigned int written = 0;

    time(&t); //need to profile it
    written = snprintf((logger.ring + offset), MAX_BUFF, "%s%.24s: ", get_mode(message_level), ctime(&t));
    //Whoops, 24 is hardcoded - its the length of ctime() without \n\0

    *left_len = MAX_BUFF - written;

    return (logger.ring + offset + written);
}

void lilog_flush(size_t start, size_t finish);

void append_log(unsigned int index) {
    int avaliable = 0;
    int old_count = 0;
    int new_count = 0;

    do {
        old_count = logger.last_printed;
        avaliable = (index > old_count) ? index - old_count :
                        BUFF_COUNT - old_count + index;

        if(avaliable != FLUSH_COUNT)
            return;

        new_count = (avaliable + old_count) % BUFF_COUNT;

    } while(!__sync_bool_compare_and_swap(&(logger.last_printed), old_count, new_count));

    lilog_flush(old_count, new_count);
}

#define FLUSH_WAIT 10000000

void lilog_flush(size_t start, size_t until) {
    size_t sz = 0;
    int err = 0;
    struct timespec t = {0, FLUSH_WAIT};
    char FLUSH_BUFF[MAX_BUFF * FLUSH_COUNT] = {0};

    while(!__sync_bool_compare_and_swap(&(logger.flush_busy), 0, 1))
        nanosleep(&t, &t);

    if(start <= until) {
        for(size_t i = start; i < until; i++)
            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * MAX_BUFF);
    }
    else {
        for(size_t i = start; i < BUFF_COUNT; i++)
            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * MAX_BUFF);

        for(size_t i = 0; i < until; i++)
            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * MAX_BUFF);

    }

    err = write(logger.output_fd, FLUSH_BUFF, sz);
    if(err == -1) {
        fprintf(stderr, "Attached logfile is dead. Unable to write into it\n");
        logger.flush_busy = 0;
        return;
    }

    logger.flush_busy = 0;
}

void lilog(int loglevel, char* str, ...) {
    int left_len = 0;
    int written = 0;
    va_list ap;
    unsigned int index = get_index();
    char* tmp = prepare_log(loglevel, index, &left_len);


    va_start(ap, str);
    written = vsnprintf(tmp, left_len, str, ap);
    va_end(ap);

    if((written > 0) && (left_len - written > 0))
        snprintf(tmp + (size_t)written, left_len - written, "\n");

    append_log(index);
;}


void hello_msg(char** argv) {
    char buff[MAX_BUFF] = {0};
    snprintf(buff, MAX_BUFF,  "Process #%d, named %s started logging", getpid(), argv[0]);
    lilog(INFO, buff);
}

#define DEFAULT_PATH "/tmp"

void init_logger(char** argv, char* path){
    char BUFF[MAX_BUFF] = {0};

    logger.ring = (char*)calloc(sizeof(char), MAX_BUFF * BUFF_COUNT);

    if(path) {
        logger.output_fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 00666);
        if(logger.output_fd == -1) {
            fprintf(stderr, "Cannot open logfile. Trying to write into standart logfile\n");
        } else {
            hello_msg(argv);
            return;
        }
    }

    snprintf(BUFF, MAX_BUFF, DEFAULT_PATH "/%s.%d.log", argv[0] + 2, getpid());

    logger.output_fd = open(BUFF, O_WRONLY | O_CREAT | O_APPEND, 00666);
    if(logger.output_fd == -1) {
        fprintf(stderr, "Cannot open logfile."
                        "Truly bad things happened! aborting\n");
        exit(-1);
    } else {
        hello_msg(argv);
        return;
    }

    return;
}

void lilog_finish(char** argv) {
    char buff[MAX_BUFF] = {0};
    snprintf(buff, MAX_BUFF,  "Process #%d, named %s succesfully finished logging", getpid(), argv[0]);
    lilog(INFO, buff);

    int index = get_index();
    lilog_flush(logger.last_printed, index);

    close(logger.output_fd);
}
