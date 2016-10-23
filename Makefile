all: DataLink

DataLink: DataLink.o termiosManip.o LinkLayer.o
	gcc DataLink.o termiosManip.o LinkLayer.o -o DataLink
	
DataLink.o:
	gcc -c DataLink.c
termiosManip.o:
	gcc -c termiosManip.c
LinkLayer.o:
	gcc -c LinkLayer.c
clean:
	rm  DataLink DataLink.o termiosManip.o LinkLayer.o
