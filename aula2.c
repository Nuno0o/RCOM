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
#define EMISSOR 0
#define RECEPTOR 1


volatile int STOP=FALSE;
int fd;

int llopen(int porta, char flag)
{

	char set[5] = 0x7e0303007e;
	char ua[5] = 0x7e0307047e;

	switch(flag){

	case EMISSOR:

		//escreve SET
		int len = sizeof(set);	

		int res = 0;
		while (len > res){
			int n = write(fd, set + res, len - res); 
			if ( !n ) break;
			res += n;
		}
	
		if ( res < len ) {
			exit(-1);
		} 


		//lê caracteres
		res = 0;
		char buf[255];
		int uteis = 0;
   		while (STOP==FALSE) {       /* loop for input */
     	res = read(fd,buf+uteis,255);   /* returns after 5 chars have been input */
      //buf[res]=0;               /* so we can printf... */
     	uteis+= res;
		if(buf[0] = 0x7e)
     	 if(buf[uteis-1] == '\0')
         printf(":%x:%d\n", buf,uteis);
	
	int nw=0;
	int len=res+1;
	while(len>nw){
		int n;
		n=write(fd,buf+nw,len-nw);
		if(!n)
		break;
		nw+=n;
	}
	if(nw<len){
	perror("error");
	exit(1);}

      if (buf[0]=='z') STOP=TRUE;
    }
		

				

	break;

	case RECEPTOR:

	break;

	}

	

	

	return 0;
}

int main(int argc, char** argv)
{
    int c, res;
    struct termios oldtio,newtio;
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

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



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

	// read line from stdin

	if (fgets(buf, 255, stdin) == NULL){
		perror("null string");
		exit(-1);
	}

	


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

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guião 
  */



   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}

