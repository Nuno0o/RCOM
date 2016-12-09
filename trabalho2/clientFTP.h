#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>

int connectSocket(const char* ip, int port);
int ftpConnect(const char* ip, int port);
int ftpWrite(int sockfd,char * buff,int size);
int ftpRead(int sockfd,char * buff,int size);
int ftpDisconnect(int sockfd);
int ftpToPassive(int sockfd);
int ftpWriteCmdAndReadReplay(int sockfd, char* cmd, char* replyBuf, char* args);
int ftpToRetr(int sockfd, char* path);
int ftpDownload(int sockfd, char* path);
int ftpLogin(int sockfd, char* username, char* pwd);
