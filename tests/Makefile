CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
PTHREAD = -lpthread
EXE = hashmap thread

all : $(EXE)

hashmap : hashmap.c
	$(CC) $(CFLAGS) hashmap.c $(PKGCONFIG) -o hashmap_exe

thread : thread.c
	$(CC) $(CFLAGS) thread.c $(PTHREAD) -o thread_exe 

cleanall:
	rm -rf *~ $(ALL)
