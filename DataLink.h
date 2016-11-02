#ifndef DATALINK_H
#define DATALINK_H



typedef enum { DATA_TRAMA, CONTROL_TRAMA } TramaType;

typedef struct {
    int tipo;
    unsigned char* trama;
    int length;
} Trama;

typedef struct{
    int rr_sent;
    int rej_sent;
    int errors;
    int n_timeOuts;
}Statistics;


void atende_alarm(int signo);
void desativa_alarm(void);
int llopen(int flag);
//Retorna numero de bytes enviados, ou -1 se exceder o tempo limite
int llwrite(int fd, unsigned char* buf, int length);
//Retorna numeros de bytes lidos, ou -1 se receber disc(fica em loop ate receber uma trama valida ou disc)
int llread(int fd, unsigned char* buf);
int llclose(int fd, int flag);
int stuffData(unsigned char* buf, int arraySize,unsigned char* newBuf);
int destuff(unsigned char* buf, int arraySize,unsigned char* newBuf);
int writeToFd(int filed,unsigned char* buf,int length, TramaType type);
Trama* receiveTrama(int fd);
int writeTramaToFd(int fd, unsigned char* trama, int length, int requiresStuffing);
void freeTrama(Trama* trama);
void printStatsReceiver();
void printStatsTransmitter();

#endif
