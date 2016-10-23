#ifndef DEFINES_H
#define DEFINES_H

#define BAUDRATE B9600

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0
#define RECEIVER 1

//Alarme
#define ALARM_NOT_AWAKE 0
#define ALARM_AWAKE 1

//Formato das tramas
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_RR 0x05
#define C_REJ 0x01
#define C_DISC 0x0B
#define C_UA 0x07
#define ESCAPE 0x7d
#define C_DATA 0x00
#define CALC 0x20

//Estados da trama
#define START -1
#define FLAG_RCV 0
#define A_RCV 1
#define C_RCV 2
#define BCC_RCV 3
#define STOP_ST 4

//Tipo de trama
#define TRAMA_ERROR -1
#define TRAMA_SET 0
#define TRAMA_UA 1
#define TRAMA_RR 2
#define TRAMA_REJ 3
#define TRAMA_DISC 4

#endif
