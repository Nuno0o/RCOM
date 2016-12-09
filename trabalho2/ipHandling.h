#ifndef IPHANDLING_H
#define IPHANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char* initIp (parsedURL* url);

#ENDIF
