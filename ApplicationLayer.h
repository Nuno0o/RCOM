#ifndef APPLICATIONLAYER_H
#define APPLICATIONLAYER_H

int receiveFile();
int sendFile();

File * loadFile();
//Envia informaçao sobre ficheiro(tamanho e nome)
int sendControl(int fd,int c,File* file);
//Preenche file com informaçao fornecida
int receiveControl(int fd,int c, File* file);
//Envia pacote de data
int sendData(int fd,int seq,int foffset,int nbyte,File* file);
//Recebe pacote de data
int receiveData(int fd,int seq,File* file);

#endif
