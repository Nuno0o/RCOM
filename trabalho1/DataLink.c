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

unsigned char SET[CONTROL_TRAMA_SIZE] = {FLAG,A,C_SET, A ^ C_SET,FLAG};
unsigned char UA[CONTROL_TRAMA_SIZE] = {FLAG,A,C_UA,A ^ C_UA,FLAG};
unsigned char DISC[CONTROL_TRAMA_SIZE] = {FLAG, A, C_DISC, A ^ C_DISC, FLAG};
unsigned char RR0[CONTROL_TRAMA_SIZE] = {FLAG, A, C_RR0, A ^ C_RR0, FLAG};
unsigned char RR1[CONTROL_TRAMA_SIZE] = {FLAG, A, C_RR1, A ^ C_RR1, FLAG};
unsigned char REJ0[CONTROL_TRAMA_SIZE] = {FLAG, A, C_REJ0, A ^ C_REJ0, FLAG};
unsigned char REJ1[CONTROL_TRAMA_SIZE] = {FLAG, A, C_REJ1, A ^ C_REJ1, FLAG};

int flag_alarm = ALARM_NOT_AWAKE;
int conta_alarm = 0;
int num = 1;

Statistics stats;


// ----------------------------------------------------------
// ------------------- ALARM MANAGEMENT ---------------------
// ----------------------------------------------------------

void atende_alarm(int signo){
	flag_alarm = ALARM_AWAKE;
	conta_alarm++;
	if(conta_alarm != Llayer->timeout+1)
		printf("Retrying %d...\n",conta_alarm);
	stats.n_timeOuts++;
	alarm(ALARM_NOT_AWAKE);
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
	stats.rr_sent = 0;
	stats.rej_sent = 0;
	stats.errors = 0;
	stats.n_timeOuts = 0;
	// Instalar Alarme
	siginterrupt(SIGALRM, TRUE);

	// Abrir porta
	int fd = open((char*)Llayer->port, O_RDWR | O_NOCTTY );
	if (fd <0) {
		perror((char*)Llayer->port); exit(-1);
	}

	// Ativar modo canonico
	saveTermios(fd,&oldtio);
	setTermios(fd,&newtio,Llayer->baudRate);

	Trama* res;
	switch(flag){
		case TRANSMITTER:
		{
			flag_alarm = ALARM_AWAKE;
			while (conta_alarm < Llayer->numTransmissions)
			{
				//Envia trama SET
				if(flag_alarm == ALARM_AWAKE){
					writeToFd(fd,SET,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
					flag_alarm = ALARM_NOT_AWAKE;
					alarm(Llayer->timeout);
				}

				//Aguarda trama UA
				res = receiveTrama(fd);
				if(res->tipo == TRAMA_UA) {
					desativa_alarm();
					return fd;
				}
			}
			return -1;
		} break;

		case RECEIVER:
		{
			// aguardar SET
			res = receiveTrama(fd);
			if (res->tipo != TRAMA_SET) {return FAILURE;}
			// enviar UA
			writeToFd(fd,UA,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
			return fd;
		} break;
	}
	return FAILURE;
}

// --------------------------- LL WRITE ------------------------------
Trama* makeDataTrama(unsigned char *buf,int length,int toggle){
	Trama* ret = (Trama*)malloc(sizeof(Trama));
	ret->trama = NULL;
	if(toggle == 0) ret->tipo = TRAMA_I_0; else ret->tipo = TRAMA_I_1;
	ret->length = length + INF_TRAMA_SIZE;

	//malloc(FLAG + A + C + BCC1 + DATA + BCC2 + FLAG)
	ret->trama = (unsigned char*)malloc(length + INF_TRAMA_SIZE);
	//Coloca byte 1 FLAG
	ret->trama[0] = FLAG;
	//Coloca byte 2 A
	ret->trama[1] = A;
	//Coloca byte 3 C_I_0 OU C_I_1 (L(s))
	if(toggle == 0) {
		ret->trama[2] = C_I_0;
	}
	else {
		ret->trama[2] = C_I_1;
	}
	//BCC
	ret->trama[3] = ret->trama[1] ^ ret->trama[2];
	//DATA
	memcpy(ret->trama+4,buf,length);
	//BCC2 = XOR de todos os elementos de data
	ret->trama[4+length] = 0;
	int j = 0;
	for(j = 0;j < length;j++) {
		ret->trama[4+length] ^= buf[j];
	}
	//FLAG 2
	ret->trama[4+length+1] = FLAG;
	return ret;

}

int llwrite(int fd, unsigned char* buf, int length){
	int receivedStop = FALSE;

	Trama* dataTrama;
	while (!receivedStop && conta_alarm < Llayer->numTransmissions){
		//Construcao da trama de dados a enviar
		dataTrama = makeDataTrama(buf,length,Llayer->ls);
		//Envio da trama
		writeToFd(fd,dataTrama->trama,dataTrama->length,DATA_TRAMA);
		//Liberta memoria
		freeTrama(dataTrama);
		//Aguarda resposta
		Trama* received;
		flag_alarm = ALARM_NOT_AWAKE;
		while (flag_alarm == ALARM_NOT_AWAKE){
			alarm(Llayer->timeout);

			received = receiveTrama(fd);

			//Se receber rej correspondente ao L(s) atual, reenvia mensagem(bytesSent nao incrementa, logo a mesma e enviada)
			if (((received->tipo == TRAMA_REJ1) && (Llayer->ls == 0)) || ((received->tipo == TRAMA_REJ0) && (Llayer->ls == 1))){
				stats.rej_sent++;
				desativa_alarm();
				freeTrama(received);
				break;
			}
			//Se receber rr correspondente ao L(s) atual,altera L(s) e retorna
			else if (((received->tipo == TRAMA_RR0) && (Llayer->ls == 1)) || ((received->tipo == TRAMA_RR1) && (Llayer->ls == 0))){
				stats.rr_sent++;
				desativa_alarm();
				Llayer->ls = !(Llayer->ls);
				receivedStop = TRUE;
				freeTrama(received);
				return length;
				break;
			}
			//Caso contrario continua a espera de mensagens
			else {
				freeTrama(received);
				continue;
			}
		}
	}
	desativa_alarm();
	return FAILURE;
}

// --------------------------- LL READ -------------------------------
int llread(int fd, unsigned char* buf){
	int receivedStop = FALSE;
	int ret = -1;
	Trama* tramaRecebida;
	while(!receivedStop){
	  tramaRecebida = receiveTrama(fd);
		//Se detetar erro no bcc1 nao faz nada
		if(Llayer->sendRandomRejs){
			int shouldISend = rand() % 4;
			if(shouldISend == 3){
				tramaRecebida->tipo = TRAMA_ERROR2;
			}
		}
		if(tramaRecebida->tipo == TRAMA_DISC){
			return -1;
		}
		else if(tramaRecebida->tipo == TRAMA_ERROR){
			stats.errors++;
		}
		//Se detetar erro no bcc2 envia REJ
		else if(tramaRecebida->tipo == TRAMA_ERROR2){


			stats.rej_sent++;

			if(Llayer->ls == 1){
				writeToFd(fd,REJ0,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
			}else writeToFd(fd,REJ1,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);

		}else if(tramaRecebida->tipo == TRAMA_I_0 || tramaRecebida->tipo == TRAMA_I_1){
			stats.rr_sent++;
			if(tramaRecebida->tipo == TRAMA_I_0){
				writeToFd(fd,RR1,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				Llayer->ls = 1;
			}else {
				writeToFd(fd,RR0,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				Llayer->ls = 0;
			};
			receivedStop = TRUE;
			//Copiar trama para buffer
			memcpy(buf,tramaRecebida->trama+TRAMA_DATA_OFFSET,tramaRecebida->length-INF_TRAMA_SIZE);
			ret = tramaRecebida->length-INF_TRAMA_SIZE;
		}
		freeTrama(tramaRecebida);
	}
	return ret;
}

// --------------------------- LL CLOSE ------------------------------
int llclose(int fd, int flag){
	Trama* res;
	switch(flag){

		case TRANSMITTER:
		{
			flag_alarm = ALARM_AWAKE;
			while (conta_alarm < Llayer->numTransmissions)
			{
				if(flag_alarm == ALARM_AWAKE){
					writeToFd(fd,DISC,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
					flag_alarm = ALARM_NOT_AWAKE;
					alarm(Llayer->timeout);
				}
				res = receiveTrama(fd);
				if(res->tipo == TRAMA_DISC) {
					desativa_alarm();
					if (writeToFd(fd,UA,CONTROL_TRAMA_SIZE,CONTROL_TRAMA) != FAILURE) {return SUCCESS;}
				}
			}
			return FAILURE;
		} break;


		case RECEIVER:
		{
			// aguardar DISC
			res = receiveTrama(fd);
			if (res->tipo == TRAMA_DISC){
				// enviar DISC
				writeToFd(fd,DISC,CONTROL_TRAMA_SIZE,CONTROL_TRAMA);
				res = receiveTrama(fd);
				if (res->tipo != TRAMA_UA) return FAILURE;
			}else {return FAILURE;}
			return SUCCESS;
		} break;
	}
	return FAILURE;
}


int writeToFd(int filed, unsigned char* buf, int length, TramaType type){
	switch (type){
		case DATA_TRAMA: return writeTramaToFd(filed, buf, length, TRUE); break;
		case CONTROL_TRAMA: return writeTramaToFd(filed, buf, length, FALSE); break;
		default: break;
	}
	return -1;
}

int writeTramaToFd(int fd, unsigned char* trama, int length, int requiresStuffing){
	int sent;
  int sentLength;
	if (!requiresStuffing) {
		sentLength = length;
		sent = write(fd,trama,length);
	}
	else{
		unsigned char* newBuf = (unsigned char*)malloc((MAX_SIZE+INF_TRAMA_SIZE)*2);//Tamanho maximo da trama depois de stuffing
		sentLength = stuffData(trama,length,newBuf);
		sent = write(fd,newBuf,sentLength);
		free(newBuf);
	}
	fsync(fd);

	if(sent != sentLength)
	perror("Message not correctly sent\n");
	return sent == sentLength;
}

/**
* something something write comments please
*/
Trama* receiveTrama(int fd){
	int tramaOffset = 0;
	unsigned char lastByte;
	int state = START;
	unsigned char* buff = (unsigned char*)malloc(MAX_SIZE*2);
	Trama* ret = (Trama*)malloc(sizeof(Trama));
	ret->trama = NULL;

	while(state != STOP_ST){
		int nbytes = read(fd,&lastByte,1);
		if(nbytes != 1){
			ret->tipo = TRAMA_ERROR;
			return ret;
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
			}//Se a trama for do tipo I, continua a receber ate receber flag
			else if(lastByte != FLAG && (buff[2] == C_I_0 || buff[2] == C_I_1)){
				buff[tramaOffset] = lastByte;
				tramaOffset++;
			}
			break;
		}
	}
	//Tramas de data
	if (buff[2] == C_I_0 || buff[2] == C_I_1){
		//Cria novo buffer para guardar data depois de destuff
		unsigned char* newBuff = (unsigned char*)malloc(MAX_SIZE*2);
		tramaOffset = destuff(buff,tramaOffset,newBuff);
		free(buff);
		ret->trama = newBuff;
		//Testa se bcc2 e valido
		unsigned char bcc2 = newBuff[tramaOffset-2];
		unsigned char bcc2test = 0;
		int j;
		for(j = 4;j < tramaOffset-2;j++){
			bcc2test ^= newBuff[j];
		}
		if(bcc2 != bcc2test){
			ret->tipo = TRAMA_ERROR2;
			return ret;
		}

		if(newBuff[2] == C_I_0)ret->tipo = TRAMA_I_0;
		else ret->tipo = TRAMA_I_1;
		ret->length = tramaOffset;
		return ret;
	}

	//Tramas de controlo
	ret->length = CONTROL_TRAMA_SIZE;
	ret->trama = buff;
	if(buff[2] == SET[2]){
		ret->tipo = TRAMA_SET;
	}else if(buff[2] == UA[2]){
		ret->tipo = TRAMA_UA;
	}else if (buff[2] == RR0[2]) {
		ret->tipo = TRAMA_RR0;
	}else if (buff[2] == RR1[2]) {
		ret->tipo = TRAMA_RR1;
	}else if (buff[2] == REJ0[2]) {
		ret->tipo = TRAMA_REJ0;
	}else if (buff[2] == REJ1[2]) {
		ret->tipo = TRAMA_REJ1;
	}else	if (buff[2] == DISC[2]){
		ret->tipo = TRAMA_DISC;
	}else ret->tipo = TRAMA_ERROR;
	return ret;
}

int stuffData(unsigned char* buf, int arraySize,unsigned char* newBuf){
	// copy flag
	newBuf[0] = buf[0];
	// j itera em newBuf, i em buf
	int i = 1,j = 0;
	for (i = 1; i < arraySize-1; i++){
		j++;
		if (buf[i] == FLAG){
			newBuf[j] = ESCAPE;
			newBuf[j + 1] = FLAG ^ CALC;
			j++;
		}
		else if (buf[i] == ESCAPE){
			newBuf[j] = ESCAPE;
			newBuf[j + 1] = ESCAPE ^ CALC;
			j++;
		}else newBuf[j] = buf[i];
	}
	newBuf[j+1] = buf[arraySize-1];
	//j+2 e o tamanho do novo array
	return j+2;
}

int destuff(unsigned char* buf, int arraySize,unsigned char* newBuf){
	// copy flag
	newBuf[0] = buf[0];
	// Valor para iterar em newBuf
	int i = 1,j = 0;
	for (i = 1; i < arraySize-1; i++){
		j++;
		if (buf[i] == ESCAPE && buf[i+1] == (FLAG ^ CALC)){
			newBuf[j] = FLAG;
			i++;
		}
		else if (buf[i] == ESCAPE && buf[i+1] == (ESCAPE ^ CALC)){
			newBuf[j] = ESCAPE;
			i++;
		}else newBuf[j] = buf[i];
	}
	newBuf[j+1] = buf[arraySize-1];
	//j+2 e o tamanho do novo array
	return j+2;

}

void freeTrama(Trama* trama){

	free(trama->trama);
	trama->trama = NULL;
	free(trama);
}

void printStatsReceiver(){
	printf("RR packets: %d\nREJ packets: %d\nErrors: %d\n",stats.rr_sent,stats.rej_sent,stats.errors);
}
void printStatsTransmitter(){
	printf("RR packets: %d\nREJ packets: %d\nTimeOuts: %d\n",stats.rr_sent,stats.rej_sent,stats.n_timeOuts);
}
