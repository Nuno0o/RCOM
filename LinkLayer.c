#include <stdlib.h>
#include "LinkLayer.h"

LinkLayer* createLinkLayer(char* port, int baudRate, int sequenceNumber, int timeout, int numTransmissions){

  LinkLayer* retorno = (LinkLayer*) malloc(sizeof(LinkLayer));
  retorno->port = port;
  retorno->baudRate = baudRate;
  retorno->sequenceNumber = sequenceNumber;
  retorno->timeout = timeout;
  retorno->numTransmissions = numTransmissions;

  return retorno;

}
