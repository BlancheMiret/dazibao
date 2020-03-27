CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
EXEC = tlv

all : $(EXEC)

tlv : tlv.c
	$(CC) $(CFLAGS) tlv.c -o tlv_exe $(CRYPTO)

cleanall:
	rm -rf *~ $(ALL)
