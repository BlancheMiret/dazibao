CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXEC = tlv neighbour test_tlv_manager

all : $(EXEC)

tlv : tlv_manager.o tlv.o dazibao.o neighbour.o
	$(CC) -o tlv tlv_manager.o tlv.o dazibao.o neighbour.o $(CRYPTO) $(PKGCONFIG) -std=gnu99

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c $(PKGCONFIG) -std=gnu99

tlv_manager.o : tlv_manager.c
	$(CC) $(CFLAGS) -o tlv_manager.o -c tlv_manager.c -std=gnu99

dazibao.o : dazibao.c tlv.h neighbour.h
	$(CC) $(CFLAGS) -o dazibao.o -c dazibao.c $(PKGCONFIG) -std=gnu99

neighbour.o : neighbour.c neighbour.h
	$(CC) $(CFLAGS) -o neighbour.o -c neighbour.c $(PKGCONFIG) -std=gnu99

neighbour : neighbour.c neighbour.h
	$(CC) $(CFLAGS) neighbour.c test_neighbour.c $(PKGCONFIG) -o neighbour_exe

test_tlv_manager : tlv_manager.c tlv_manager.h
	$(CC) $(CFLAGS) tlv_manager.c test_tlv_manager.c -o test_tlv_exe $(CRYPTO)


cleanall:
	rm -rf *~ $(ALL)
