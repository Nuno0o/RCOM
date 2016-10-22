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
#include <signal.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TRANSMITTER 0
#define RECEIVER 1

//Formato das tramas
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_RR 0x05
#define C_REJ 0x01
#define C_DISC 0x0B
#define C_UA 0x07
#define ESCAPE 0x7d
#define C_DATA 0x00
#define CALC 0x20

//Estados da trama
#define START -1
#define FLAG_RCV 0
#define A_RCV 1
#define C_RCV 2
#define BCC_RCV 3
#define STOP_ST 4

//Tipo de trama
#define TRAMA_ERROR -1
#define TRAMA_SET 0
#define TRAMA_UA 1


volatile int STOP=FALSE;
struct termios oldtio,newtio;

char SET[6] = {FLAG,A,C_SET, A ^ C_SET,FLAG,'\0'};
char UA[6] = {FLAG,A,C_UA,A ^ C_UA,FLAG};

int flag_alarm = 0, conta_alarm = 0,num = 1;
char buf[10];

//---------------------- PROTOTIPOS
int saveTermios(int filed,struct termios * ter);
int setTermios(int filed,struct termios * ter);
int resetTermios(int filed,struct termios * ter);
int writeToFd(int filed,char* buf,int length);
//-1 = erro, 0 = set, 1 = ua
int receiveTrama(int fd);

// -------------------- DEFINICOES

void atende_alarm(int signo){
	flag_alarm = 1;
	conta_alarm++;
	alarm(0);
	printf("Alarm %d \n", conta_alarm);
}

void desativa_alarm(void) {
	alarm(0);
	conta_alarm = 0;
	flag_alarm = 0;
}

int saveTermios(int filed, struct termios * ter){
	if ( tcgetattr(filed,ter) == -1) {
      perror("tcgetattr");
      exit(-1);
    }
}
int resetTermios(int filed, struct termios * ter){
	if ( tcsetattr(filed,TCSANOW,ter) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
}

int setTermios(int filed,struct termios * ter){
	bzero(ter, sizeof(*ter));
    ter->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    ter->c_iflag = IGNPAR;
    ter->c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    ter->c_lflag = 0;

    ter->c_cc[VTIME]    = 0;   /* inter-character timer unused */
    ter->c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

 	 /*
  	  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  	  leitura do(s) pr�ximo(s) caracter(es)
  	*/

    tcflush(filed, TCIOFLUSH);

    if ( tcsetattr(filed,TCSANOW,ter) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    //printf("New termios structure set\n");
}

int llopen(char* porta, int flag)
{
	siginterrupt(SIGALRM, 1);
	char buf[255];
	// abrir porta
	int fd = open(porta, O_RDWR | O_NOCTTY );
    	if (fd <0) {perror(porta); exit(-1); }
	// modo canonico
	saveTermios(fd,&oldtio);
	setTermios(fd,&newtio);

	switch(flag){

	case TRANSMITTER:
	{


		printf("Trama SET enviada\n");
		int res = 0;
		while (conta_alarm < 3 && res != TRAMA_UA)
		{
			writeToFd(fd,SET,5);
			flag_alarm = 0;
			while (flag_alarm == 0 && res != TRAMA_UA) {
				alarm(3);
				res = receiveTrama(fd);
			}
			if(res == TRAMA_UA) desativa_alarm();
		}
		// ...


		return fd;
	}
	break;

	case RECEIVER:
	{
		// aguardar SET
		int res = receiveTrama(fd);
		// enviar UA
		writeToFd(fd,UA,5);
		printf("Trama UA enviada\n");
		// ...
		return fd;

	}

	break;

	}

	return -1;
}

char* stuffData(char* buf, int arraySize){
	//Stuffing needed?
	int i;
	int newArraySize = arraySize;
	//check for FLAG or ESC existance. first and last bytes are FLAG...
	for (i = 1; i < arraySize - 1; i++){
			// check for FLAG or ESCAPE
			if (buf[i] == FLAG || buf[i] == ESCAPE) newArraySize++;

	}

	char* newBuf = (char*) malloc(newArraySize);
	memcpy(newBuf,buf,arraySize);
	// create the new array
	for (i = 1; i < newArraySize; i++){
			if (newBuf[i] == FLAG){
					memmove(newBuf+i+1,newBuf+i,arraySize-i);
					newBuf[i] = ESCAPE;
					newBuf[i + 1] = FLAG ^ CALC;
					i++;
			}
			else if (newBuf[i] == ESCAPE){
					memmove(newBuf+i+1,newBuf+i,arraySize-i);
					newBuf[i] = ESCAPE;
					newBuf[i + 1] = ESCAPE ^ CALC;
					i++;
			}
	}

	return newBuf;
}

/*ui destuff(unsigned char** buf, ui bufSize) {
	int i;
	for (i = 1; i < bufSize - 1; ++i) {
		if ((*buf)[i] == ESCAPE) {
			memmove(*buf + i, *buf + i + 1, bufSize - i - 1);

			bufSize--;

			(*buf)[i] ^= 0x20;
		}
	}

	*buf = (unsigned char*) realloc(*buf, bufSize);

	return bufSize;
}*/

char* destuff(unsigned char* buf, int arraySize){
	//Stuffing needed?
	int i;
	//check for FLAG or ESC existance. first and last bytes are FLAG...

	// create the new array
	for (i = 1; i < newArraySize; i++){
			if (newBuf[i] == ESCAPE){
					memmove(newBuf+i,newBuf+i+1,arraySize-i-1);
					if (newBuf[i] == FLAG ^ CALC) newBuf[i] = FLAG;
					else newBuf[i] = ESCAPE;
			}
	}
	return newBuf;
}

int writeToFd(int filed,char* buf, int length){

	int sent;

	sent = write(filed,buf,length);

	fsync(filed);

	if(sent != length)
		perror("Message not correctly sent\n");

	return sent == length;

}

/**
* something something write comments please ;D
*/
int receiveTrama(int fd){
	char buff[255];
	int tramaOffset = 0;
	char lastByte;
	int state = START;

	while(state != STOP_ST){
		int nbytes = read(fd,&lastByte,1);
		if(nbytes != 1){
			return -1;
		}

		switch(state){
			case START:
				if(lastByte == FLAG){
					buff[tramaOffset] = lastByte;
					tramaOffset++;
					state = FLAG_RCV;
				}
				break;
			case FLAG_RCV:
				if(lastByte == A){
					buff[tramaOffset] = lastByte;
					tramaOffset++;
					state = A_RCV;
				}else
				if(lastByte == FLAG){
					tramaOffset = 1;
					state = FLAG_RCV;
				}else {
					tramaOffset = 0;
					state = START;
				}
				break;
			case A_RCV:
				if(lastByte == FLAG){
					tramaOffset = 1;
					state = FLAG_RCV;
				}else{
					buff[tramaOffset] = lastByte;
					tramaOffset++;
					state = C_RCV;
				}
				break;
			case C_RCV:
				if(lastByte == buff[1] ^ buff[2]){
					buff[tramaOffset] = lastByte;
					tramaOffset++;
					state = BCC_RCV;
				}else
				if(lastByte == FLAG){
					tramaOffset = 1;
					state = FLAG_RCV;
				}else{
					tramaOffset = 0;
					state = START;
				}
				break;
			case BCC_RCV:
				if(lastByte == FLAG){
					buff[tramaOffset] = lastByte;
					tramaOffset++;
					state = STOP_ST;
				}
				break;
		}
	}
	if(buff[2] == C_SET){
		return 0;
	}else if(buff[2] == C_UA){
		return 1;
	}else return -1;
}


int main(int argc,  char** argv)
{
	signal(SIGALRM, atende_alarm);

    char buf[255];

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
	/*
		Open serial port device for reading and writing and not as controlling tty
		because we don't want to get killed if linenoise sends CTRL-C.
	*/
    int fd = llopen(argv[1],transorres);
		if (fd <0) {perror(argv[1]); exit(-1); }

	// read line from stdin



    resetTermios(fd, &oldtio);

    close(fd);

    return 0;
}
