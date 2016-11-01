#ifndef APPLICATIONLAYER_H
#define APPLICATIONLAYER_H

int receiveFile();
int sendFile();

File * loadFile();
//Envia informaçao sobre ficheiro(tamanho e nome)
int sendControl(int fd,int c,File* file);
//Preenche file com informaçao fornecida
int receiveControl(int fd,int c, File* file);

int sendData(int fd,File* file);

#endif
