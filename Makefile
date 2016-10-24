all: DataLink

DataLink: DataLink.o termiosManip.o LinkLayer.o File.o
	gcc DataLink.o termiosManip.o LinkLayer.o File.o -o DataLink
	
DataLink.o:
	gcc -c DataLink.c
termiosManip.o:
	gcc -c termiosManip.c
LinkLayer.o:
	gcc -c LinkLayer.c
File.o:
	gcc -c File.c
clean:
	rm  DataLink DataLink.o termiosManip.o LinkLayer.o File.o

.PHONY:all
