CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXEC = tlv neighbour


all : tlv.o dazibao.o neighbour.o 
	$(CC) -o tlv tlv.o dazibao.o neighbour.o $(CRYPTO) $(PKGCONFIG) -std=gnu99

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c $(PKGCONFIG) -std=gnu99

dazibao.o : dazibao.c tlv.h neighbour.h
	$(CC) $(CFLAGS) -c dazibao.c $(PKGCONFIG) -o dazibao.o -std=gnu99

neighbour.o : neighbour.c neighbour.h
	$(CC) $(CFLAGS) -o neighbour.o -c neighbour.c $(PKGCONFIG) -std=gnu99


cleanall:
	rm -rf *~ $(ALL)

