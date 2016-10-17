/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TRANSMITTER 0
#define RECEIVER 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define BCC_SET A ^ C_SET
#define BCC_UA  A ^ C_UA


volatile int STOP=FALSE;
int fd;
struct termios oldtio,newtio;

char SET[5] = {FLAG,A,C_SET,BCC_SET,FLAG};
char UA[5] = {FLAG,A,C_UA,BCC_UA,FLAG};

//---------------------- PROTOTIPOS
int saveTermios();
int setTermios();
int resetTermios();
int writeToFd(char* buf);
int readFromFd(char* buf);

// -------------------- DEFINICOES

int llopen(char* porta, int flag)
{
	// abrir porta
	fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); exit(-1); }
	
	// modo canonico
	saveTermios(&oldtio);
	setTermios(&newtio,fd);

	switch(flag){

	case TRANSMITTER:
	{
		// enviar SET
		writeToFd(SET);
		
		// receber UA
		readFromFd(UA);
		// ...

		return fd;
	}
	break;

	case RECEIVER:
	{
		// aguardar SET
		
		// enviar UA
		writeToFd(UA);
		// ...
	}

	break;

	}

	return -1;
}
int saveTermios(){
	if ( tcgetattr(fd,&oldtio) == -1) { 
      perror("tcgetattr");
      exit(-1);
    }
}
int resetTermios(){
	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
}

int setTermios(){
	bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

 	 /* 
  	  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
  	  leitura do(s) próximo(s) caracter(es)
  	*/

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
}

int writeToFd(char* buf){

	int len, nw;
	
	len = strlen(buf);

	buf[len-1]='\0';

	nw = 0;
	while (len > nw){
		int n;
		n = write(fd, buf + nw, len - nw); 
		if ( !n ) break;
		nw += n;
	}
	
	if ( nw < len ) { 
		perror ("number written is wrong");
		exit(-1);
	} 
	
	return nw;
}
	
int readFromFd(char* buf){
	bzero(&buf,sizeof(buf));
    int res;
	int uteis = 0;
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf+uteis,255);   /* returns after 1 chars have  been input */
      uteis+= res;
      if(buf[uteis-1] == '\0'){
         printf(":%s:%d\n", buf,uteis);
		 STOP = TRUE;
	  }
	}
	
	return uteis;
}

int main(int argc, char** argv)
{
    int c, res;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


     fd = open(argv[1], O_RDWR | O_NOCTTY );
     if (fd <0) {perror(argv[1]); exit(-1); }

	saveTermios(&oldtio);

	setTermios(&newtio,fd);

	// read line from stdin

	if (fgets(buf, 255, stdin) == NULL){
		perror("null string");
		exit(-1);
	}

	writeToFd(buf);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}

