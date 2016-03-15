#ifndef MAX_BUFF
#define MAX_BUFF 128
#endif // MAX_BUFF

#ifndef BUFF_COUNT
#define BUFF_COUNT 128
#endif // BUFF_COUNT

#ifndef FLUSH_COUNT
#define FLUSH_COUNT 16
#endif // FLUSH_COUNT

struct Logger {
    ñhar* ring;
    unsigned int tail;
    int output_fd;
} logger = {0};


unsigned int get_tail() {
    int old_tail = 0, tail = 0;
    do {
        old_tail = logger.tail;
        tail = old_tail + 1;
    } while(!__sync_bool_compare_and_swap(&(logger.tail), old_tail, tail))

    return old_tail;
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
        return KWHT GET_MODE_STR(INFO) RESET
    case WARNING :
        return KBLU GET_MODE_STR(WARNING) RESET
    case ERROR :
        return KYEL GET_MODE_STR(ERROR) RESET
    case DEBUG :
        return KRED GET_MODE_STR(DEBUG) RESET
    default :
        return KMAG GET_MODE_STR(UNKNOWN) RESET
    }
}

char* prepare_log(int message_level, unsigned int tail, int* left_len) {
    void* index = (tail % BUFF_COUNT) * MAX_BUFF;
    time_t t = {0};
    unsigned int written = 0;

    time(&t); //need to profile it
    written = snprintf((logger.ring + index), MAX_BUFF, "%s %.24s: ", get_mode(message_level), ctime(t));
    //Whoops, 24 is hardcoded - its the length of ctime() without \n\0

    *left_len = MAX_BUFF - written;

    return (logger.ring + index + written);
}

void append_log(unsigned int tail) {
    if(!(tail % FLUSH_COUNT) && tail >= FLUSH_COUNT) {
        unsigned int start = (tail - FLUSH_COUNT) % BUFF_COUNT;
        unsigned int finish = (tail - 1) % BUFF_COUNT;
        int sz = 0;
        int err = 0;

        if(start < finish) {
            sz = FLUSH_COUNT * MAX_BUFF;
            err = write(logger.output_fd, logger.ring + start * MAX_BUFF, sz);
            if(err == -1) {
                fprintf(STDERR_FILENO, "Attached logfile is dead. Unable to write into it\n");
                return;
            }
        }
        else {
            sz = (BUFF_COUNT - start + 1) * MAX_BUFF;
            err = write(logger.output_fd, logger.ring + start * MAX_BUFF, sz);
            if(err == -1) {
                fprintf(STDERR_FILENO, "Attached logfile is dead. Unable to write into it\n");
                return;
            }

            sz = finish * MAX_BUFF;
            err = write(logger.output_fd, logger.ring[0], sz);
            if(err == -1) {
                fprintf(STDERR_FILENO, "Attached logfile is dead. Unable to write into it\n");
                return;
            }

        }

    }

//    printf(logger.ring[index % BUFF_COUNT]);
}

void my_log(char* str) {
    int left_len = 0;
    unsigned int tail = get_tail();
    char* tmp = prepare_log(LogLevel, tail, &left_len);
    snprintf(tmp, left_len, "%s\n", str);

    append_log(tail);
;}


void hello_msg(char** argv) {
    char buff[MAX_BUFF] = {0};
    snprintf(buff, MAX_BUFF,  "Process #%d, named %s started logging \n", getpid(), argv[0]);
    my_log(buff);
}

void init_logger(char** argv, char* path){

    logger.ring = (char*)calloc{}

    if(path) {
        logger.fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 00444);
        if(logger.fd == -1) {
            fprintf(STDERR_FILENO, "Cannot open logfile. Trying to write into standart logfile\n");
        } else {
            hello_msg(argv);
            return;
        }
    }

    logger.fd = 0;

}
