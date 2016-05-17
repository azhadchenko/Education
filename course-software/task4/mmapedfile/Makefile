CC = gcc
CFLAGS = -Wall -std=gnu99
STATICLIBS =  mmaped_file.c inverted_index.c chunk_manager.c
EXECUTABLE = test.c

all : makeo
	ar rcs mapped_file.a $(STATICLIBS:.c=.o)
	$(CC) $(CFLAGS) test.c -L. mapped_file.a

makeo : $(STATICLIBS)
	$(CC) $(CFLAGS) -o mmaped_file.o    -c mmaped_file.c
	$(CC) $(CFLAGS) -o inverted_index.o -c inverted_index.c
	$(CC) $(CFLAGS) -o chunk_manager.o  -c chunk_manager.c


clean :
	rm $(STATICLIBS:.c = .o)


