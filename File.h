#ifndef FILE_H
#define FILE_H

#include "defines.h"

typedef struct {
    int             fd;
    unsigned char*           fileMode;
    FILE*           fileStream;
    long int        fileSize;
    unsigned char*           fileName;
} File;

File* initFile(char* fileName,char* fileMode);

#endif
