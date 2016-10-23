#ifndef TERMIOSMANIP_H
#define TERMIOSMANIP_H
#include <termios.h>

int saveTermios(int filed,struct termios * ter);
int setTermios(int filed,struct termios * ter);
int resetTermios(int filed,struct termios * ter);
#endif
