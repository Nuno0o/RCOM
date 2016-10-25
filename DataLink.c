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

volatile int STOP=FALSE;

struct termios oldtio,newtio;

char SET[CONTROL_TRAMA_SIZE] = {FLAG,A,C_SET, A ^ C_SET,FLAG};
char UA[CONTROL_TRAMA_SIZE] = {FLAG,A,C_UA,A ^ C_UA,FLAG};
char DISC[CONTROL_TRAMA_SIZE] = {FLAG, A, C_DISC, A ^ C_DISC, FLAG};

int flag_alarm = ALARM_NOT_AWAKE;
int conta_alarm = 0;
int num = 1;

char buf[10];

LinkLayer* Llayer;

// ----------------------------------------------------------
// ------------------- ALARM MANAGEMENT ---------------------
// ----------------------------------------------------------

void atende_alarm(int signo){
	flag_alarm = ALARM_AWAKE;
	conta_alarm++;
	alarm(ALARM_NOT_AWAKE);
	printf("Alarm %d \n", conta_alarm);
}

void desativa_alarm(void) {
	alarm(ALARM_NOT_AWAKE);
	conta_alarm = 0;
	flag_alarm = ALARM_NOT_AWAKE;
}

// ------------------------------------------------------------
// ---------------------- LL OPERATIONS -----------------------
// ------------------------------------------------------------

// ------------------------- LL OPEN -------------------------
int llopen(int flag)
{
	// Instalar Alarme
	siginterrupt(SIGALRM, TRUE);

	// Abrir porta
	int fd = open(Llayer->port, O_RDWR | O_NOCTTY );
	if (fd <0) {
		perror(Llayer->port); exit(-1);
	}

	// Ativar modo canonico
	saveTermios(fd,&oldtio);
	setTermios(fd,&newtio);

	char buff[MAX_SIZE];

	switch(flag){
		case TRANSMITTER:
		{
			printf("Trama SET enviada\n");
			int res = 0;
			while (conta_alarm < Llayer->numTransmissions && res != TRAMA_UA)
			{
				writeToFd(fd,SET,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				flag_alarm = ALARM_NOT_AWAKE;
				while (flag_alarm == ALARM_NOT_AWAKE && res != TRAMA_UA) {
					alarm(Llayer->timeout);
					res = receiveTrama(fd, buff);
				}
				if(res == TRAMA_UA) {
					printf("Trama UA recebida!\n");
					desativa_alarm();
				}
			}
			return fd;
		} break;
		case RECEIVER:
		{
			// aguardar SET
			int res = receiveTrama(fd, buff);
			if (res != TRAMA_SET) return FAILURE;
			// enviar UA
			writeToFd(fd,UA,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
			printf("Trama UA enviada\n");
			return fd;
		} break;
	}
	return FAILURE;
}

// --------------------------- LL WRITE ------------------------------
char* makeTrama(int trama,char *buf,int length,int toggle){
	char* ret;

	switch(trama){
		case TRAMA_I:
		{
			//malloc(FLAG + A + C + BCC1 + DATA + BCC2 + FLAG)
			ret = (char*)malloc(length + INF_TRAMA_SIZE);
			//Coloca byte 1 FLAG
			ret[0] = FLAG;
			//Coloca byte 2 A
			ret[1] = A;
			//Coloca byte 3 C_I_0 OU C_I_1 (L(s))
			if(toggle == 0) {
				ret[2] = C_I_0;
			}
			else {
				ret[2] = C_I_1;
			}
			//BCC
			ret[3] = ret[1] ^ ret[2];
			//DATA
			memcpy(ret+4,buf,length);
			//XOR de todos os elementos de data
			ret[4+length] = 0;
			int j = 0;
			for(j = 0;j < length;j++) {
				ret[4+length] ^= buf[j];
			}
			//FLAG 2
			ret[4+length+1] = FLAG;
			return ret;
		}
			break;
		default:
			break;
	}
	return NULL;
}

int llwrite(int fd, char* buf, int length){
	char buff[MAX_SIZE];
	int receivedStop = FALSE;
	//Numero de bytes ja enviados
	int bytesSent = 0;
	//L(s)
	int ls = 0;
	while (!receivedStop && conta_alarm < Llayer->numTransmissions && bytesSent < length){
		//Bytes a enviar na trama atual
		int toSend;
		if(length-bytesSent < 256) toSend = length-bytesSent;
		else toSend = 256;
		//Construcao da trama de dados a enviar
		char* dataTrama = makeTrama(TRAMA_I,buf+bytesSent,toSend,ls);
		//Envio da trama
		writeToFd(fd,dataTrama,toSend+ INF_TRAMA_SIZE,DATA_TRAMA);
		//Liberta memoria
		free(dataTrama);

		int received = 0;
		while (flag_alarm == ALARM_NOT_AWAKE){
			alarm(Llayer->timeout);
			received = receiveTrama(fd, buff);
			//Se receber rej correspondente ao L(s) atual, reenvia mensagem(bytesSent nao incrementa, logo a mesma e enviada)
			if (((received == TRAMA_REJ0) && (ls == 0)) || ((received == TRAMA_REJ1) && (ls == 1))){
				desativa_alarm();
				break;
			}
			//Se receber rr correspondente ao L(s) atual, incrementa o numero de bytes enviados e passa para envio de proxima trama
			else if (((received == TRAMA_RR0) && (ls == 0)) || ((received == TRAMA_RR1) && (ls == 1))){
				desativa_alarm();
				if(ls == 0) ls = 1;else ls = 0;
				bytesSent+=toSend;
			}
			//Caso contrario continua a espera de mensagens
			else continue;
		}
	}
	if(bytesSent == length)
		return SUCCESS;
	return FAILURE;
}

// --------------------------- LL READ -------------------------------
int llread(int fd, char* buf){
	return -1;
}

// --------------------------- LL CLOSE ------------------------------
int llclose(int fd, int flag){

	char buff[MAX_SIZE];

	switch(flag){
		case TRANSMITTER:
		{
			printf("Trama DISC enviada\n");
			int res = 0;
			while (conta_alarm < Llayer->numTransmissions && res != TRAMA_DISC)
			{
				writeToFd(fd,DISC,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				flag_alarm = ALARM_NOT_AWAKE;
				while (flag_alarm == ALARM_NOT_AWAKE && res != TRAMA_DISC) {
					alarm(Llayer->timeout);
					res = receiveTrama(fd, buff);
				}
				if(res == TRAMA_DISC) {
					desativa_alarm();
					if (writeToFd(fd,UA,CONTROL_TRAMA_SIZE,CONTROL_TRAMA) != FAILURE) return SUCCESS;
				}
			}
			return FAILURE;
		} break;
		case RECEIVER:
		{
			// aguardar DISC
			int res = receiveTrama(fd, buff);
			if (res == TRAMA_DISC){
				// enviar DISC
				writeToFd(fd,DISC,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				printf("Trama DISC enviada\n");
				res = receiveTrama(fd, buff);
				if (res != TRAMA_UA) return FAILURE;
			}
			return SUCCESS;
		} break;
	}
	return FAILURE;
}


int writeToFd(int filed, char* buf, int length, TramaType type){
	switch (type){
		case DATA_TRAMA: return writeTramaToFd(filed, buf, length, TRUE); break;
		case CONTROL_TRAMA: return writeTramaToFd(filed, buf, length, FALSE); break;
		default: break;
	}
	return -1;
}

int writeTramaToFd(int fd, char* trama, int length, int requiresStuffing){
	int sent;

	if (!requiresStuffing) sent = write(fd,trama,length);
	else sent = write(fd,stuffData(trama, length),length);
	fsync(fd);

	if(sent != length)
	perror("Message not correctly sent\n");
	return sent == length;
}

/**
* something something write comments please ;D
*/
int receiveTrama(int fd, char* buff){
	int tramaOffset = 0;
	char lastByte;
	int state = START;

	while(state != STOP_ST){
		int nbytes = read(fd,&lastByte,1);
		printf("%d\n",nbytes);
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
			if(lastByte == (buff[1] ^ buff[2])){
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
		return TRAMA_SET;
	}else if(buff[2] == C_UA){
		return TRAMA_UA;
	}else if (buff[2] == C_RR0) {
		return TRAMA_RR0;
	}else if (buff[2] == C_RR1) {
		return TRAMA_RR1;
	}else if (buff[2] == C_REJ0) {
		return TRAMA_REJ0;
	}else if (buff[2] == C_REJ1) {
		return TRAMA_REJ1;
	}else	if (buff[2] == C_DISC){
		return TRAMA_DISC;
	} return TRAMA_ERROR;
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

char* destuff(char* buf, int arraySize){
	//Stuffing needed?
	int i;
	//check for FLAG or ESC existance. first and last bytes are FLAG...

	// create the new array
	for (i = 1; i < arraySize; i++){
		if (buf[i] == ESCAPE){
			memmove(buf+i,buf+i+1,arraySize-i-1);
			if (buf[i] == (FLAG ^ CALC)) buf[i] = FLAG;
			else buf[i] = ESCAPE;
		}
	}

	return buf;
}

int main(int argc,  char** argv)
{
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
	/*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
	*/
	Llayer = createLinkLayer(argv[1], BAUDRATE, 0, 3, 3);
	int fd = llopen(transorres);
	if (fd <0) {perror(argv[1]); exit(-1); }
	llclose(fd,transorres);
	resetTermios(fd, &oldtio);
	close(fd);

	return 0;
}
