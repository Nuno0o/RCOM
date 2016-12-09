#ifndef PARSEURL_H
#define PARSEURL_H

#include "defines.h"

typedef struct{
    char username[MAX_SIZE];
	char password[MAX_SIZE];
	char host[MAX_SIZE];
	char path[MAX_SIZE];
    char ip[MAX_SIZE];
    int port;
}parsedURL;

parseURL(char* url,parsedURL* parsedURL);

#endif