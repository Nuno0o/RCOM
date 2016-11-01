#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "DataLink.h"
#include "defines.h"
#include "termiosManip.h"
#include "LinkLayer.h"
#include "File.h"
#include "ApplicationLayer.h"
#include "libgen.h"

LinkLayer* Llayer;


// -----------------------------------------------------------------------------
// ---------------------------------- RECEIVER ---------------------------------
// -----------------------------------------------------------------------------
int receiveFile(){
  File* file = (File*)malloc(sizeof(File));
  int fd;
  printf("Awaiting connection from transmitter...\n");
  fd = llopen(RECEIVER);
  if (fd < 0){
    perror("Connection Attempt failed. Exiting...\n");
    exit(FAILURE);
  }else printf("Connection established\n");

  if(receiveControl(fd,CONTROL_START,file) < 0){
    printf("Error receiving CONTROL_START:timed out\n");
  }else printf("CONTROL_START received successfully.\nReceiving file \"%s\"(%d bytes)\n",file->fileName,file->fileSize);

  file->fileStream = fopen(file->fileName,"wb");

  //numero na sequencia a ser enviada
  int seq = 0;
  int totalReceived = 0;

  while(totalReceived < file->fileSize){
    int received = receiveData(fd,seq,file);
    //Se for repetido
    if(received == 0){
      continue;
    }else if(received < 0){
      printf("Error receiving DATA,exiting...\n");
      exit(1);
    }
    totalReceived += received;
    seq++;
    printf("Received: %d/%d bytes\n",totalReceived,file->fileSize);

  }

  if(fclose(file->fileStream) != 0){
    printf("Error closing file.\n");
    exit(1);
  }else printf("File Received successfully\n");

  if(receiveControl(fd,CONTROL_END,file) < 0){
    printf("Error receiving CONTROL_END:timed out\n");
    exit(1);
  }else printf("CONTROL_END received successfully.\n");

  if(llclose(fd,TRANSMITTER) < 0){
    perror("Closing attempt failed. Exiting...\n");
    exit(FAILURE);
  }else printf("Successfully closed connection.\n");
  return SUCCESS;
}

// -----------------------------------------------------------------------------
// ---------------------------------- TRANSMITTER ------------------------------
// -----------------------------------------------------------------------------
int sendFile(){

  File* file = loadFile();
  int fd;
  printf("Attempting to connect...\n");
  fd = llopen(TRANSMITTER);
  if (fd < 0){
    perror("Connection attempt failed (timed out). Exiting...\n");
    exit(1);
  }else printf("Connection established.\n");

  if(sendControl(fd,CONTROL_START,file) < 0){
    printf("Error sending CONTROL_START:timed out\n");
    exit(1);
  }else printf("CONTROL_START sent successfully.\n");

  //numero na sequencia a ser enviada
  int seq = 0;
  int totalSent = 0;

  while(totalSent < file->fileSize){
    int toSend;
    if(file->fileSize - totalSent > DATA_DEFAULT_SIZE){
      toSend = DATA_DEFAULT_SIZE;
    }else toSend = file->fileSize - totalSent;
    if(sendData(fd,seq % DATA_DEFAULT_SIZE,toSend,totalSent,file) < 0){
      printf("Error sending DATA:timed out\n");
      exit(1);
    }else{
      totalSent += toSend;
      seq++;
      printf("Sent: %d/%d bytes\n",totalSent,file->fileSize);
    }
  }

  if(sendControl(fd,CONTROL_END,file) < 0){
    printf("Error sending CONTROL_END:timed out\n");
    exit(1);
  }else printf("CONTROL_END sent successfully.\n");

  if(fclose(file->fileStream) != 0){
    printf("Error closing file.\n");
    exit(1);
  }else printf("File Sent successfully\n");

  if(llclose(fd,TRANSMITTER) < 0){
    perror("Closing attempt failed. Exiting...\n");
    exit(FAILURE);
  }else printf("Successfully closed connection.\n");
  return SUCCESS;
}

int sendData(int fd,int seq,int nbyte,int foffset,File* file){
  unsigned char buf[MAX_SIZE*4];
  int i = 0;

  fseek(file->fileStream,foffset,SEEK_SET);
  unsigned char* tempBuf = (unsigned char*) malloc(nbyte);
  int nread = fread(tempBuf,sizeof(unsigned char),nbyte,file->fileStream);
  fseek(file->fileStream,foffset,SEEK_SET);

  buf[i++] = CONTROL_DATA;
  buf[i++] = (unsigned char)seq;
  int l1 = nread % 256;
  int l2 = nread / 256;
  buf[i++] = (unsigned char)l2;
  buf[i++] = (unsigned char)l1;
  memcpy(buf+i,tempBuf,nread);
  i+= nread;
  free(tempBuf);

  int sent = llwrite(fd,buf,i);

  if(sent < 0)
    return FAILURE;
  return SUCCESS;
}

int receiveData(int fd,int seq,File* file){
  unsigned char buf[MAX_SIZE*4];
  int messageSize = llread(fd,buf);
  if(messageSize < 0)
    return FAILURE;

  int i = 0;

  if (buf[i++] != CONTROL_DATA){
    return FAILURE;
  }
  //Verifica se o pacote recebido é o proximo item da sequencia
  if(buf[i++] != (unsigned char)seq){
    if(buf[i++] == (unsigned char)seq -1)
      return 0;
    else return FAILURE;
  }
  int l2 = (int)buf[i++];
  int l1 = (int)buf[i++];
  int nbyte = l1+256*l2;
  unsigned char* tempBuf = (unsigned char*)malloc(nbyte);
  memcpy(tempBuf,buf+i,nbyte);

  fwrite(tempBuf,sizeof(unsigned char),nbyte,file->fileStream);

  return nbyte;

}


// -----------------------------------------------------------------------------
// ------------------------ FILE MANAGEMENT ------------------------------------
// ----------------------------------------------------------------------------
File * loadFile(){
  File* file;
  unsigned char * fileName = (unsigned char*)malloc(MAX_SIZE);
  printf("Insert file name: ");
  scanf("%s",fileName);
  file = initFile(fileName,"rb");
  if(file == NULL){
    printf("Error opening file \"%s\", exiting...\n",fileName);
    exit(1);
  }else printf("File loaded with success.\n");
  printFileProps(file);

  return file;
}

int sendControl(int fd,int c,File* file){

  if(c != CONTROL_START && c != CONTROL_END){
    return FAILURE;
  }
  int i = 0;
  //Buffer da mensagem a enviar
  unsigned char buf[MAX_SIZE];
  buf[i++] = c;

  //Colocaçao do TYPE de  tamanho do ficheiro
  buf[i++] = CONTROL_TYPE_SIZE;
  //Coloca tamanho do ficheiro num buffer de unsigned chars(cada unsigned char e um numero)
  unsigned char sizeInArray[20];
  sprintf(sizeInArray,"%ld",file->fileSize);
  //Colocaçao do Length de tamanho de ficheiro
  buf[i++] = strlen(sizeInArray);
  int j;
  //Colocaçao do tamanho de ficheiro
  for(j = 0;j < strlen(sizeInArray);j++){
    buf[i++] = sizeInArray[j];
  }

  //Colocaçao do TYPE de nome de ficheiro
  buf[i++] = CONTROL_TYPE_NAME;
  //Coloca nome de ficheiro num buffer
  unsigned char* fileNameWithoutDir = basename(file->fileName);
  //Colocaçao de LENGTH do nome de ficheiro
  buf[i++] = (unsigned char)strlen(fileNameWithoutDir);
  //Colocaçao do nome de ficheiro
  for(j = 0;j < strlen(fileNameWithoutDir);j++){
    buf[i++] = fileNameWithoutDir[j];
  }
  if(c == CONTROL_START)
    printf("Sending file \"%s\"(%ld bytes)\n",fileNameWithoutDir,file->fileSize);

  if(c == CONTROL_END)
    printf("Finished sending file \"%s\"(%ld bytes)\n",fileNameWithoutDir,file->fileSize);

  int sent = llwrite(fd,buf,i);
  if(sent < 0)
    return FAILURE;
  return SUCCESS;
}

int receiveControl(int fd, int c,File* file){
  unsigned char buf[MAX_SIZE];
  int messageSize = llread(fd,buf);
  if(messageSize < 0)
    return FAILURE;

  int i = 0;

  if(buf[i++] != (unsigned char)c)
    return FAILURE;
  //Cada ciclo le um parametro TLV
  while(i < messageSize){
    int currType = (int)buf[i++];
    if(currType == CONTROL_TYPE_SIZE){
      int length = (int)buf[i++];
      unsigned char* value = (unsigned char*)malloc(length+1);
      memcpy(value,buf+i,length);
      value[length] = 0;
      long int size = -1;
      sscanf(value,"%ld",&size);
      free(value);
      file->fileSize = size;
      i+= length;
    }
    else if(currType == CONTROL_TYPE_NAME){
      int length = (int)buf[i++];
      unsigned char* value = (unsigned char*)malloc(length);
      memcpy(value,buf+i,length);
      file->fileName = value;
      i+= length;
    }
  }
  return SUCCESS;
}



int main(int argc, unsigned char** argv){
  signal(SIGALRM, atende_alarm);

	if ( (argc < 3 || argc > 7) ||
	((strcmp("/dev/ttyS0", argv[1])!=0) &&
	(strcmp("/dev/ttyS1", argv[1])!=0) )) {
		printf("Usage:\tnserial SerialPort MODE\n\tex: nserial /dev/ttyS1 [TRANSMITTER|RECEIVER] [BAUDRATE] [MAX_ATTEMPTS] [TIMEOUT] [MAX_TRAMA_SIZE]\n");
		exit(1);
	}

	int transorres = -1;

	if(strcmp("TRANSMITTER",argv[2]) == 0){
		transorres = TRANSMITTER;
	}else
	if(strcmp("RECEIVER",argv[2]) == 0){
		transorres = RECEIVER;
	}else{
		printf("Third argument has to be TRANSMITTER or RECEIVER");
		exit(1);
	}

  Llayer = createLinkLayer(argv[1], BAUDRATE_DEF, 0, ATTEMPTS_DEF, TIMEOUT_DEF, DATA_DEFAULT_SIZE);

  if (argc >= 4){
    int baudRate = atoi(argv[3]);
    if (setBaudrate(baudRate, Llayer) != SUCCESS) return FAILURE;
  }
  if (argc >= 5){
    int maxAttempts = atoi(argv[4]);
    if (setMaxAttempts(maxAttempts,Llayer) != SUCCESS) return FAILURE;
  }
  if (argc >= 6){
    int timeout = atoi(argv[5]);
    if (setTimeout(timeout, Llayer) != SUCCESS) return FAILURE;
  }
  if (argc >= 7){
    int maxSize = atoi(argv[4]);
    if (setMaxSize(maxSize,Llayer) != SUCCESS) return FAILURE;
  }

  if(transorres == TRANSMITTER){
		sendFile();
	}else{
		receiveFile();
	}
  return 0;
}
