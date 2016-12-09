#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "parseURL.h"
#include "defines.h"
//Inicializa struct
void initParsed(parsedURL * parsed){
    parsed->username = 0;
    parsed->password = 0;
    parsed->ip = 0;
    parsed->host = 0;
    parsed->path = 0;
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
        if(userPassSep != null && userPassSep < atSign){

            int usernameSize = userPassSep - currUrl;
            parsedURL->username = (char*)malloc(usernameSize+1);
            strncpy(parsedURL->username,currUrl,usernameSize);
            currUrl += usernameSize;

            currUrl += 1;

            int passwordSize = atSign - currUrl;
            parsedURL->password = (char*)malloc(passwordSize+1);
            strncpy(parsedURL->password,currUrl,passwordSize);
            currUrl += passwordSize;
        }else{
            int usernameSize = atSign - currUrl;
            parsedURL->username = (char*)malloc(usernameSize+1);
            strncpy(parsedURL->username,currUrl,usernameSize);
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
        parsedURL->path = (char*)malloc(pathSize+1);
        strncpy(parsedURL->path,pathSep,pathSize);
    }else{
        parsedURL->path = (char*)malloc(1);
        parsedURL->path = "";
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
        parserURL->port = atoi(port);
    }else{
        parsedURL->port = 21;
    }
    //Host parse
    {
        int temp = firstPointer(portSep,pathSep);
        int hostEnd = firstPointer(temp,endOfString);
        int hostSize = hostEnd - currUrl;
        if(hostSize == 0){
            printf("No host specified\n");
            return -1;
        }
        parsedURL->host = (char*)malloc(hostSize+1);
        strncpy(parsedURL->host,currUrl,hostSize);
    }
    return 0;

}