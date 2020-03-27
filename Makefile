CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
EXEC = tlv

all : $(EXEC)

tlv : tlv.o dazibao.o
	$(CC) -o tlv tlv.o dazibao.o $(CRYPTO)

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c

dazibao.o : dazibao.c tlv.h
	$(CC) $(CFLAGS) -o dazibao.o -c dazibao.c 


cleanall:
	rm -rf *~ $(ALL)
