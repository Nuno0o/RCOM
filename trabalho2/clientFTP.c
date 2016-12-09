/*      (C)2000 FEUP  */

#include "clientFTP.h"

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"
#define SIZE 512

int connectSocket(const char* ip, int port){

	int	sockfd;
	struct	sockaddr_in server_addr;
	int	bytes;

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
		printf("error connecting to socket.\n");
		return -1;
	}
	else{
		printf("connect succesful");
	}
	return sockfd;
}

int ftpDisconnect(int sockfd){
	if(ftpWrite(sockfd,"QUIT","")!=0){
		close(sockfd);
		return 1;
	}
	char replyBuf[SIZE];
	if(ftpRead(sockfd,replyBuf,SIZE)!=0){
		close(sockfd);
		return 1;
	}
	close(fd);
	return 0;
}

int ftpToPassive(int sockfd) {
	if (ftpWrite(sockfd, "PASV", "") != 0){
		close(sockfd);
		return 1;
	}
	char replyBuf[SIZE];

	if (ftpRead(sockfd,replyBuf, SIZE) != 0){
			close(sockfd);
			return 1;
	}

	int ip_1, ip_2, ip_3, ip_4, port_1, port_2;

	if ((sscanf(replyBuf, "Going Passive: %u,%u,%u,%u,%u,%u,%u"), &ip1,&ip2,&ip3,&ip4,&port1,&port2) < 0){
		printf("ERROR");
		return 1;
	}

	sprintf(ip, "%u.%u.%u.%u", ip1, ip2, ip3, ip4);

	int port=250*port1+port2;
	if(connectSocket(&(sockfd),ip,port)!=0){
		close(sockfd);
		return -1;
	}
	return 0;
}

int ftpWrite(int sockfd,char * buff,int size){
	int n;
	if((n=write(sockfd,buff,size))==size){
	   	printf("write succeded");
		return 0;
	}
	else
	return -1;
}

int ftpRead(int sockfd,char * buff,int size){
	File *socketPtr;
	if((socketPtr=fdopen(sockfd,"r"))==NULL){
		printf("fdopen failed");
		return -1;
	}
	do{
		memset(buff,0,size);
			buff=fgets(buff,size,socketptr);
		}while(!(buff[0]>='1' && buff[0]<='5'));
	}

	return 0;
}
