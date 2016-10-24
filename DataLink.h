#ifndef DATALINK_H
#define DATALINK_H

typedef enum { DATA_TRAMA, CONTROL_TRAMA } TramaType;

void atende_alarm(int signo);
void desativa_alarm(void);
int llopen(int flag);
char* stuffData(char* buf, int arraySize);
char* destuff(unsigned char* buf, int arraySize);
int writeToFd(int filed,char* buf,int length, TramaType type);
int receiveTrama(int fd, char* buff);

#endif
