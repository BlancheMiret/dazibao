CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
EXEC = tlv

all : $(EXEC)

tlv : tlv.o dazibao.o
	$(CC) -o tlv tlv.o dazibao.o $(CRYPTO) -std=gnu99

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c -std=gnu99

dazibao.o : dazibao.c tlv.h
	$(CC) $(CFLAGS) -o dazibao.o -c dazibao.c -std=gnu99


cleanall:
	rm -rf *~ $(ALL)
