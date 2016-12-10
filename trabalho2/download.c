#include <stdio.h>
#include "clientFTP.h"
#include "parseURL.h"
#include "ipHandling.h"
#include "defines.h"


int main(int argc, char* argv[]) {

  if (argc != 2) {
    printf("Usage: download url");
    return 1;
  }

  parsedURL url;
  initParsed(&url);

  if (parseURL(argv[1], &url) != 0){
    return -1;
  }

	printf("\n\nURL parsed correctly. Proceeding...\n");

  char* IP;
  IP = initIp(&url);

  if (IP == NULL) {
    printf("Error converting to IP.\n\n");
    return -1;
  }
    printf("IP got. IP = %s\n\n", IP);

  int sockfd = -1;
  int datafd = -1;

  sockfd = ftpConnect(IP, url.port);
  if ( sockfd < 0)
  {
    printf("Error estabilishing connection (ftpConnect failed).\n\n");
    return 1;
  }
  else {
 	printf("Socket file descriptor at: %d. Connection successful!\n\n", sockfd);
  }

	printf("Time to login.\nUsername got: %s\nPassword got: %s\n", url.username, url.password);

    if (ftpLogin(sockfd, url.username, url.password) < 0){
      printf("Attempted to login, but failed.\n\n");
      return 1;
    }
	
	printf("Login successful, so far so good!\n\n");

  if ((datafd = ftpToPassive(sockfd)) < 0){
    printf("Attempted to enter passive mode, but failed.\n\n");
    return 1;
  }

	printf("Now starting download of requested file.\n");

  if (ftpDownload(datafd, sockfd, url.path) < 0){
    printf("Download operation failed!\n");
    return 1;
  }

	printf("Download complete. Disconnecting...\n\n");

  if (ftpDisconnect(sockfd) <0 ){
    printf("Attempted to disconnect, but failed.\n\n");
    return 1;
  }

	printf("Disconnected.\n\n");

  return 0;

}
