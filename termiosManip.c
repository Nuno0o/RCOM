#include "termiosManip.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "defines.h"

int saveTermios(int filed, struct termios * ter){
  	if ( tcgetattr(filed,ter) == -1) {
        perror("tcgetattr");
        exit(-1);
      }
    return 0;
}

int resetTermios(int filed, struct termios * ter){
  	if ( tcsetattr(filed,TCSANOW,ter) == -1) {
        perror("tcsetattr");
        exit(-1);
      }
    return 0;
}

int setTermios(int filed,struct termios * ter){
	  bzero(ter, sizeof(*ter));
    ter->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    ter->c_iflag = IGNPAR;
    ter->c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    ter->c_lflag = 0;
    ter->c_cc[VTIME]    = 0;   /* inter-character timer unused */
    ter->c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
    tcflush(filed, TCIOFLUSH);
    if ( tcsetattr(filed,TCSANOW,ter) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    return 0;
}
