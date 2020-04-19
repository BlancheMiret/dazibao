CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXEC = tlv neighbour

all : $(EXEC)

tlv : tlv.o dazibao.o
	$(CC) -o tlv tlv.o dazibao.o $(CRYPTO) $(PKGCONFIG) -std=gnu99

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c $(PKGCONFIG) -std=gnu99

dazibao.o : dazibao.c tlv.h
	$(CC) $(CFLAGS) -o dazibao.o -c dazibao.c $(PKGCONFIG) -std=gnu99

neighbour : neighbour.c neighbour.h
	$(CC) $(CFLAGS) neighbour.c test_neighbour.c $(PKGCONFIG) -o neighbour_exe


cleanall:
	rm -rf *~ $(ALL)
