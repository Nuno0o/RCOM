#include <stdio.h>
#include <stdlib.h>
#include "File.h"

File* initFile(int fd, char* fileMode){
    FILE* stream;
    if ((stream = fdopen(fd, fileMode)) == NULL){
        perror("Error creating FileStruct!\n");
        return NULL;
    }
    File* retorno = (File*) malloc(sizeof(File));
    retorno->fileStream = stream;
    retorno->fd = fd;
    retorno->fileMode = fileMode;
    retorno->fileSize = lseek(retorno->fd, 0, SEEK_END) + 1;
}

long getFileOffset(File* file){
    if (file != NULL){
      return ftell(file->fileStream);
    }
    return -1;
}
