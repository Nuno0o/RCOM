/*      (C)2000 FEUP  */

#include "clientFTP.h"
#include <libgen.h>

#define SIZE 1024

int connectSocket(const char* ip, int port){

	int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	/*connect to the server*/
    	if(connect(sockfd,
	           (struct sockaddr *)&server_addr,
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}


	return sockfd;
}

int ftpConnect(const char* ip, int port){
	int sockfd=connectSocket(ip,port);


	if(sockfd<0){
		return -1;
	}
	else{
		char buff[SIZE];
		if (ftpRead(sockfd, buff, sizeof(buff)) < 0){
			return 1;		
		}
	}
	return sockfd;
}


int ftpWrite(int sockfd,char * buff,int size){
	int n;
	if((n=write(sockfd,buff,size))==size){
		return 0;
	}
	else {
		printf("... write failed ...\n");
		return -1;
	}

}

int ftpRead(int sockfd,char * buff,int size){
	FILE *socketPtr;
	if((socketPtr=fdopen(sockfd,"r"))==NULL){
		printf("Failed to open socket (fdopen())");
		return -1;
	}
	do{
		memset(buff,0,size);
			buff=fgets(buff,size,socketPtr);
		}while(!(buff[0]>='1' && buff[0]<='5'));
	return 0;
}

int ftpDisconnect(int sockfd){
	char replyBuf[SIZE];
	if( ftpWriteCmdAndReadReplay(sockfd, "QUIT", replyBuf, "") < 0)
		return 1;
	close(sockfd);
	return 0;
}

int ftpToPassive(int sockfd) {
	char replyBuf[SIZE];
	if( ftpWriteCmdAndReadReplay(sockfd, "PASV", replyBuf, "") < 0)
		return 1;

	unsigned int ip_oct1, ip_oct2, ip_oct3, ip_oct4, port_pt1, port_pt2;
	if(sscanf(replyBuf, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)\n", &ip_oct1, &ip_oct2, &ip_oct3, &ip_oct4, &port_pt1, &port_pt2) < 0)
	{
		close(sockfd);
		return 1;
	}

	char ip[ 3*4 + 4];
	sprintf(ip, "%u.%u.%u.%u", ip_oct1, ip_oct2, ip_oct3, ip_oct4);

	printf("IP at host: %s\n", ip);

	int port=256*port_pt1+port_pt2;
	
	int datafd;
	
	if((datafd = connectSocket(ip,port)) == -1){
		close(sockfd);
		return -1;
	}
	return datafd;
}

int ftpWriteCmdAndReadReplay(int sockfd, char* cmd, char* replyBuf, char* args){
	char msg[SIZE];
	sprintf(msg, "%s %s\r\n", cmd, args);
	if (ftpWrite(sockfd, msg, strlen(msg)) != 0) {
		close(sockfd);
		return 1;
	}
	if(ftpRead(sockfd, replyBuf, SIZE) != 0) {
		close(sockfd);
		return 1;
	}
	return 0;
}

int ftpToRetr(int sockfd, char* path) {
	char replyBuf[SIZE];
	if (ftpWriteCmdAndReadReplay(sockfd, "RETR", replyBuf, path) < 0)
		return 1;
	return 0;
}

int ftpDownload(int datafd, int sockfd, char* path){

	if (ftpToRetr(sockfd, path) != 0) {
		close(sockfd);
		return 1;
	}

	FILE* file;
	char* fileName;
	fileName = basename(path);


	if (!(file = fopen(fileName,"w"))){
		return 1;
	}
	
	char data[SIZE];
	int readData;

	while (( readData = read(datafd, data, sizeof(data)))){
		if (readData < 0){
			return 1;
		}
		readData = fwrite(data, readData, 1, file);
	}

	fclose(file);
	close(datafd);
	return 0;
}

int ftpLogin(int sockfd, char* username, char* pwd) {
	char replyBuf[SIZE];
	if (ftpWriteCmdAndReadReplay(sockfd, "USER", replyBuf, username) < 0)
		return 1;

	memset(replyBuf, 0, sizeof(replyBuf));

	if (ftpWriteCmdAndReadReplay(sockfd, "PASS", replyBuf, pwd) < 0)
		return 1;

	char reply[4];
	strncpy(reply, replyBuf, 3);
	reply[4] = '\0';
	char goodReply[4] = "230";

	if (strncmp(reply, goodReply, strlen(goodReply)) != 0) {
		printf("Login failed with those credentials.\n");
		return 1;	
	}

	return 0;
}
