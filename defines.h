#ifndef DEFINES_H
#define DEFINES_H

#define BAUDRATE B9600

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1
#define SUCCESS 1
#define FAILURE -1

#define TRANSMITTER 0
#define RECEIVER 1

#define MAX_SIZE 256

//Alarme
#define ALARM_NOT_AWAKE 0
#define ALARM_AWAKE 1

//Formato das tramas
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_RR0 0x05
#define C_RR1 0x85
#define C_REJ0 0x01
#define C_REJ1 0x51
#define C_DISC 0x0B
#define C_UA 0x07
#define ESCAPE 0x7d
#define CALC 0x20
#define C_I_0 0x00
#define C_I_1 0x40


#define TRAMA_I 5

//Estados da trama
#define START -1
#define FLAG_RCV 0
#define A_RCV 1
#define C_RCV 2
#define BCC_RCV 3
#define STOP_ST 4

//Tipo de trama
#define TRAMA_ERROR2 -2
#define TRAMA_ERROR -1
#define TRAMA_SET 0
#define TRAMA_UA 1
#define TRAMA_I_0 2
#define TRAMA_I_1 3
#define TRAMA_RR0 4
#define TRAMA_RR1 5
#define TRAMA_REJ0 6
#define TRAMA_REJ1 7
#define TRAMA_DISC 8


#define CONTROL_TRAMA_SIZE 5
#define INF_TRAMA_SIZE 6

#define TRAMA_DATA_OFFSET 4

//App LinkLayer
#define CONTROL_START 2
#define CONTROL_END 3
#define CONTROL_DATA 1
#define CONTROL_TYPE_NAME 1
#define CONTROL_TYPE_SIZE 0

#endif
