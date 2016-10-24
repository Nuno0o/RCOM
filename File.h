#ifndef FILE_H
#define FILE_H

#include "defines.h"

typedef struct {
    int             fd;
    char*           fileMode;
    FILE*           fileStream;
    unsigned int    fileSize;
} File;


#endif
