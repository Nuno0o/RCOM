#ifndef DATALINK_H
#define DATALINK_H

typedef enum { DATA_TRAMA, CONTROL_TRAMA } TramaType;

void atende_alarm(int signo);
void desativa_alarm(void);
int llopen(int flag);
int stuffData(char* buf, int arraySize,char* newBuf);
int destuff(char* buf, int arraySize,char* newBuf);
int writeToFd(int filed,char* buf,int length, TramaType type);
int receiveTrama(int fd, char* buff);
int writeTramaToFd(int fd, char* trama, int length, int requiresStuffing);

#endif
