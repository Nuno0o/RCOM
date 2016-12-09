all: bin/download

bin/parseURL.o: parseURL.c parseURL.h defines.h
	cc -c parseURL.c -o bin/parseURL.o -Wall

bin/ipHandling.o: ipHandling.c ipHandling.h 
	cc -c ipHandling.c -o bin/ipHandling.o -Wall

bin/clientFTP.o: clientFTP.c clientFTP.h defines.h
	cc -c clientFTP.c -o bin/clientFTP.o -Wall

bin/download.o: download.c defines.h parseURL.h ipHandling.h clientFTP.h
	cc -c download.c -o bin/download.o -Wall

bin/download: bin/parseURL.o bin/ipHandling.o bin/clientFTP.o bin/download.o
	cc -o bin/download bin/parseURL.o bin/ipHandling.o bin/clientFTP.o bin/download.o -Wall

clean:
	-rm bin/parseURL.o bin/ipHandling.o bin/clientFTP.o -rm bin/download.o

.PHONY: all clean
