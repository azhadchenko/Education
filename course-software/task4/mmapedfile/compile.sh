#! /bin/bash

gcc -m32  -D_FILE_OFFSET_BITS=64 -std=gnu99 test.c
