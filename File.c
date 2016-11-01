#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "File.h"

File* initFile(char* fileName,char* fileMode){
    FILE* stream;
    stream = fopen(fileName,fileMode);
    if (stream == NULL){
        return NULL;
    }
    File* retorno = (File*) malloc(sizeof(File));
    retorno->fd = fileno(stream);
    retorno->fileName = fileName;
    retorno->fileStream = stream;
    retorno->fileMode = fileMode;
    fseek(stream, 0, SEEK_END);
    retorno->fileSize = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    return retorno;
}

long getFileOffset(File* file){
    if (file != NULL){
      return ftell(file->fileStream);
    }
    return -1;
}
