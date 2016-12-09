#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "parseURL.h"
#include "defines.h"

//Inicializa struct
void initParsed(parsedURL * parsed){
    parsed->username = NULL;
    parsed->password = NULL;
    parsed->ip = NULL;
    parsed->host = NULL;
    parsed->path = NULL;
    parsed->port = -1;
}

char* firstPointer(char* x,char* y){
    if(x == NULL){
        return y;
    }
    if(y == NULL){
        return x;
    }
    if(x < y){
        return x;
    }else
        return y;
}

int parseURL(char* url,parsedURL* parsed){

    //Inicializar valores da struct
    initParsed(parsed);

    //Novo apontador para iterar sobre a string
    char* currUrl = url;

    //Check if header is correct
    if(strncmp(currUrl,FTP_HEADER,FTP_HEADER_SIZE) != 0){
        printf("URL doesn't have correct header.\n");
        return -1;
    }

    currUrl += FTP_HEADER_SIZE;

    //Verifica se username / password sao especificados
    char* atSign = strchr(currUrl,'@');

    if(atSign != NULL){
        char* userPassSep = strchr(currUrl,':');
        if(userPassSep != NULL && userPassSep < atSign){

            int usernameSize = userPassSep - currUrl;
            parsed->username = (char*)malloc(usernameSize+1);
            strncpy(parsed->username,currUrl,usernameSize);
            currUrl += usernameSize;

            currUrl += 1;

            int passwordSize = atSign - currUrl;
            parsed->password = (char*)malloc(passwordSize+1);
            strncpy(parsed->password,currUrl,passwordSize);
            currUrl += passwordSize;
        }else{
            int usernameSize = atSign - currUrl;
            parsed->username = (char*)malloc(usernameSize+1);
            strncpy(parsed->username,currUrl,usernameSize);
            currUrl += usernameSize;
        }
        currUrl += 1;
    }

    //Port and path separators
    char* portSep = strchr(currUrl,':');
    char* pathSep = strchr(currUrl,'/');
    char* endOfString = currUrl + strlen(currUrl);

    //Path parse
    if(pathSep != NULL){
        int pathSize = endOfString - pathSep;
        parsed->path = (char*)malloc(pathSize+1);
        strncpy(parsed->path,pathSep,pathSize);
    }else{
        parsed->path = (char*)malloc(1);
        parsed->path = "";
    }

    //Port parse
    if(portSep != NULL){
        char * portEnd = firstPointer(endOfString,pathSep);
        int portSize = portEnd - portSep - 1;
        if(portSize == 0){
            printf("No port specified after ':'\n");
            return -1;
        }
        char * port = (char*)malloc(portSize+1);
        strncpy(port,portSep+1,portSize);
        parsed->port = atoi(port);
    }else{
        parsed->port = 21;
    }

    //Host parse
    {
        char* temp = firstPointer(portSep,pathSep);
        char* hostEnd = firstPointer(temp,endOfString);
        int hostSize = (int)(hostEnd - currUrl);
        if(hostSize == 0){
            printf("No host specified\n");
            return -1;
        }
        parsed->host = (char*)malloc(hostSize+1);
        strncpy(parsed->host,currUrl,hostSize);
    }
    return 0;

}
