#ifndef PARSEURL_H
#define PARSEURL_H

#include "defines.h"

typedef struct{
      char* username;
      char* password;
      char* host;
      char* path;
      char* ip;
      int port;
}parsedURL;

//Faz parse de string com url para struct
int parseURL(char* url,parsedURL* parsed);
void initParsed(parsedURL * parsed);
char* firstPointer(char* x,char* y);

#endif
