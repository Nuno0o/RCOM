all: bin/AppLayer

bin/AppLayer: bin/DataLink.o bin/termiosManip.o bin/LinkLayer.o bin/File.o bin/ApplicationLayer.o
	cc -o bin/AppLayer bin/DataLink.o bin/termiosManip.o bin/LinkLayer.o bin/File.o bin/ApplicationLayer.o -Wall

bin/DataLink.o: DataLink.c DataLink.h File.h LinkLayer.h defines.h termiosManip.h
	cc -c DataLink.c -o bin/DataLink.o -Wall

bin/termiosManip.o: termiosManip.c termiosManip.h defines.h
	cc -c termiosManip.c -o bin/termiosManip.o -Wall

bin/LinkLayer.o: LinkLayer.c LinkLayer.h
	cc -c LinkLayer.c -o bin/LinkLayer.o -Wall

bin/File.o: File.c File.h
	cc -c File.c -o bin/File.o -Wall

bin/ApplicationLayer.o: ApplicationLayer.c ApplicationLayer.h
	cc -c ApplicationLayer.c -o bin/ApplicationLayer.o -Wall

clean:
	-rm bin/AppLayer bin/DataLink.o bin/termiosManip.o bin/LinkLayer.o bin/File.o

.PHONY: all clean
