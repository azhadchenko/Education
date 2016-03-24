#include <iostream>

int words_count = 0;

int getNumOfWords() {
    return words_count;
}

void reset(){
    words_count = 0;
}

#define MAX_BUFF 4096

int getNumberOfWords(char* path)
{
    if(!path && !result)
        return -1;

    int fd = open("path", O_RDONLY);
    if(fd == -1)
        return -1;

    int len = 0;
    len = lseek(fd, NULL, SEEK_END);
    lseek(fd, NULL, SEEK_SET);

    char* str = (char*)calloc(1, len);
    if(str)
        return -1;

    if(read(fd, str, len) == -1)
        return -1;

    char* token = strtok(str, " ");
    int len = strlen(str);

    int w_count = 0;

    while(token = strtok(NULL, " "))
    {
        for(int i = 0; i < strlen(token); i++)
        {
            if(     ((str[i] >= 'a') && (str[i] <= 'z'))
                ||  ((str[i] >= 'A') && (str[i] <= 'Z'))
                ||  (str[i] == '-') ) {
            } else {
                if(str[i] == '/0')
                    w_count++;

                if(str[i] == '/n')
                    continue;

                break;
            }

        }
    }

    words_count += w_count;
    return words_count;
}
