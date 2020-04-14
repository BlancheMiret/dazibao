CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXE = hashmap

all : $(EXE)

hashmap : hashmap.c
	$(CC) $(CFLAGS) hashmap.c $(PKGCONFIG) -o hashmap_exe

cleanall:
	rm -rf *~ $(ALL)
