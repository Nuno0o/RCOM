#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "DataLink.h"
#include "defines.h"
#include "termiosManip.h"
#include "LinkLayer.h"
#include "File.h"

LinkLayer* Llayer;

int receiveFile(){
  int fd = llopen(RECEIVER);
}
int sendFile(){
  int fd = llopen(TRANSMITTER);
}


int main(int argc,  char** argv){
  signal(SIGALRM, atende_alarm);

	if ( (argc != 3) ||
	((strcmp("/dev/ttyS0", argv[1])!=0) &&
	(strcmp("/dev/ttyS1", argv[1])!=0) )) {
		printf("Usage:\tnserial SerialPort MODE\n\tex: nserial /dev/ttyS1 TRANSMITTER\n");
		exit(1);
	}

	int transorres = -1;

	if(strcmp("TRANSMITTER",argv[2]) == 0){
		transorres = TRANSMITTER;
	}else
	if(strcmp("RECEIVER",argv[2]) == 0){
		transorres = RECEIVER;
	}else{
		printf("Third argument has to be TRANSMITTER or RECEIVER");
		exit(1);
	}
  Llayer = createLinkLayer(argv[1], BAUDRATE, 0, 3, 3);
  if(transorres == TRANSMITTER){
		sendFile();
	}else{
		receiveFile();
	}
}
