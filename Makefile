CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
CRYPTO = -lcrypto
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXEC = tlv test_neighbour test_tlv_manager

all : $(EXEC)

#################### LINKING

tlv : dazibao.o tlv_manager.o tlv.o neighbour.o data_manager.o hash.o 
	$(CC) -o tlv dazibao.o tlv_manager.o tlv.o neighbour.o data_manager.o hash.o $(CRYPTO) $(PKGCONFIG) -std=gnu99

test_neighbour : test_neighbour.o neighbour.o
	$(CC) $(CFLAGS) test_neighbour.o neighbour.o $(PKGCONFIG) -o test_neighbour_exe

test_tlv_manager : test_tlv_manager.o tlv_manager.o hash.o
	$(CC) $(CFLAGS) test_tlv_manager.o tlv_manager.o hash.o -o test_tlv_exe $(CRYPTO)

test_data_manager : test_data_manager.c data_manager.o hash.o
	$(CC) $(CFLAGS) test_data_manager.c data_manager.o hash.o -o test_data_exe $(PKGCONFIG) $(CRYPTO)

###################### COMPILATION OF OBJECT FILES

tlv.o : tlv.c
	$(CC) $(CFLAGS) -o tlv.o -c tlv.c $(PKGCONFIG) -std=gnu99

dazibao.o : dazibao.c tlv_manager.h neighbour.h data_manager.h hash.h
	$(CC) $(CFLAGS) -o dazibao.o -c dazibao.c $(PKGCONFIG) -std=gnu99

test_neighbour.o : test_neighbour.c neighbour.h
	$(CC) $(CFLAGS) -o test_neighbour.o -c test_neighbour.c $(PKGCONFIG)

test_tlv_manager.o : test_tlv_manager.c tlv_manager.h
	$(CC) $(CFLAGS) -o test_tlv_manager.o -c test_tlv_manager.c $(CRYPTO)

tlv_manager.o : tlv_manager.c tlv_manager.h hash.h
	$(CC) $(CFLAGS) -o tlv_manager.o -c tlv_manager.c $(PKGCONFIG) -std=gnu99

neighbour.o : neighbour.c neighbour.h
	$(CC) $(CFLAGS) -o neighbour.o -c neighbour.c $(PKGCONFIG) -std=gnu99

data_manager.o : data_manager.c data_manager.h hash.h
	$(CC) $(CFLAGS) -c data_manager.c -o data_manager.o $(PKGCONFIG)-std=gnu99

hash.o : hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c -o hash.o $(CRYPTO) $(PKGCONFIG)-std=gnu99



cleanall:
	rm -rf *~ $(ALL)
