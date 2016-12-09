#ifndef PARSEURL_H
#define PARSEURL_H

#include "defines.h"


typedef struct{
    char * username[MAX_SIZE];
	char * password[MAX_SIZE];
	char * host[MAX_SIZE];
	char * path[MAX_SIZE];
    char * ip[MAX_SIZE];
    int port;
}parsedURL;
//Faz parse de string com url para struct
int parseURL(char* url,parsedURL* parsed);

#endif