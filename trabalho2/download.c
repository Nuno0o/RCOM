#include <stdio.h>
#include "clientFTP.h"
#include "parseURL.h"
#include "ipHandling.h"
#include "defines.h"


int main(int argc, char* argv[]) {

  if (argc < 3) {
    printf("Usage: download url");
    return 1;
  }

  parsedURL* url;
  initParsed(url);

  if (parseURL(argv[1], url) != 0){
    return -1;
  }

  char* IP;
  IP = initIp(url);

  if (IP == NULL) {
    printf("Error converting to IP.");
    return -1;
  }
  else {
    printf("IP: %s", IP);
  }

  int sockfd = -1;
  sockfd = ftpConnect(IP, url->port);
  if ( sockfd < 0)
  {
    printf("Error estabilishing connection (ftpConnect failed).");
    return 1;
  }

  if ( url->username != NULL && url->password != NULL){
    if (ftpLogin(sockfd, url->username, url->password) < 0){
      printf("Attempted to login, but failed.");
      return 1;
    }
  }

  if (ftpToPassive(sockfd) < 0){
    printf("Attempted to enter passive mode, but failed.");
    return 1;
  }

  if (ftpDownload(sockfd, url->path) < 0){
    printf("Download operation failed!");
    return 1;
  }

  if (ftpDisconnect(sockfd) <0 ){
    printf("Attempted to disconnect, but failed.");
    return 1;
  }

  return 0;

}
