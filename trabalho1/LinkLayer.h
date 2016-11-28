#ifndef LINKLAYER_H
#define LINKLAYER_H

#define MAX_SIZE 256

typedef struct {
    unsigned char* port;
    int baudRate;
    unsigned int sequenceNumber;
    unsigned int timeout;
    unsigned int numTransmissions;
    unsigned char frame[MAX_SIZE];
    int ls;
    unsigned int maxSize;
    unsigned int sendRandomRejs;
} LinkLayer;

LinkLayer* createLinkLayer(unsigned char* port, int baudRate, int sequenceNumber,unsigned int timeout,unsigned int numTransmissions,  unsigned int maxSize);
int setBaudrate(int baudrate, LinkLayer* linkLayer);
int setMaxAttempts(int maxAttempts, LinkLayer* linkLayer);
int setTimeout(int timeout, LinkLayer* linkLayer);
int setMaxSize(int maxSize, LinkLayer* linkLayer);

extern LinkLayer* Llayer;

#endif
