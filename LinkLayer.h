#ifndef LINKLAYER_H
#define LINKLAYER_H

#define MAX_SIZE 256

typedef struct {
    char* port;
    int baudRate;
    unsigned int sequenceNumber;
    unsigned int timeout;
    unsigned int numTransmissions;
    char frame[MAX_SIZE];
    int ls;
} LinkLayer;

LinkLayer* createLinkLayer(char* port, int baudRate, int sequenceNumber, int timeout, int numTransmissions);

#endif
