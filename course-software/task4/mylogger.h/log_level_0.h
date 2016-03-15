struct Logger {

};


void inner_log(int message_level, char* str) {

    printf(str);

}


void mylog(char* str) {
    inner_log(LOG_INFO, str);
}


