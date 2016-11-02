#include <stdlib.h>
#include "LinkLayer.h"
#include "defines.h"

LinkLayer* createLinkLayer(unsigned char* port, int baudRate, int sequenceNumber,unsigned int timeout,unsigned int numTransmissions, unsigned int maxSize){

  LinkLayer* retorno = (LinkLayer*) malloc(sizeof(LinkLayer));
  retorno->port = port;
  retorno->baudRate = baudRate;
  retorno->sequenceNumber = sequenceNumber;
  retorno->timeout = timeout;
  retorno->numTransmissions = numTransmissions;
  retorno->ls = 0;
  retorno->maxSize = maxSize;
  return retorno;
}

int setBaudrate(int baudrate, LinkLayer* linkLayer){
  switch(baudrate){
    case 0: linkLayer->baudRate = BAUDRATE0; return SUCCESS;
    case 50: linkLayer->baudRate = BAUDRATE50; return SUCCESS;
    case 75: linkLayer->baudRate = BAUDRATE75; return SUCCESS;
    case 110: linkLayer->baudRate = BAUDRATE110; return SUCCESS;
    case 134: linkLayer->baudRate = BAUDRATE134; return SUCCESS;
    case 150: linkLayer->baudRate = BAUDRATE150; return SUCCESS;
    case 200: linkLayer->baudRate = BAUDRATE200; return SUCCESS;
    case 300: linkLayer->baudRate = BAUDRATE300; return SUCCESS;
    case 600: linkLayer->baudRate = BAUDRATE600; return SUCCESS;
    case 1200: linkLayer->baudRate = BAUDRATE1200; return SUCCESS;
    case 1800: linkLayer->baudRate = BAUDRATE1800; return SUCCESS;
    case 2400: linkLayer->baudRate = BAUDRATE2400; return SUCCESS;
    case 4800: linkLayer->baudRate = BAUDRATE4800; return SUCCESS;
    case 9600: linkLayer->baudRate = BAUDRATE9600; return SUCCESS;
    case 19200: linkLayer->baudRate = BAUDRATE19200; return SUCCESS;
    case 38400: linkLayer->baudRate = BAUDRATE38400; return SUCCESS;
    default: break;
  }
  return FAILURE;
}

int setMaxAttempts(int maxAttempts, LinkLayer* linkLayer){
  if (maxAttempts > 0){
    linkLayer->numTransmissions = maxAttempts; return SUCCESS;
  }
  return FAILURE;
}

int setTimeout(int timeout, LinkLayer* linkLayer){
  if (timeout > 0){
    linkLayer->timeout = timeout; return SUCCESS;
  }
  return FAILURE;
}

int setMaxSize(int maxSize, LinkLayer* linkLayer){
  if (maxSize > 0 && maxSize <= 256){
    linkLayer->maxSize = maxSize; return SUCCESS;
  }
  return FAILURE;
}
