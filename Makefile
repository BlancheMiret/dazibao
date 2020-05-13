CC = gcc #compilateur utilisé
CFLAGS = -Wall -g -std=c11 #options de compilation
LDLIBS = -lm
#LDFLAGS = options de l'éditeur de lien
CRYPTO = -lcrypto
GLIBHEADER = `pkg-config --cflags glib-2.0`
GLIBLINK =  `pkg-config --libs glib-2.0`
PKGCONFIG = `pkg-config --cflags --libs glib-2.0`
EXEC = tlv test_neighbour_exe test_tlv_exe test_data_exe #noms des executables à générer
SOURCES = dazibao.c inondation.c maintain_neighbours.c tlv_manager.c data_manager.c neighbour.c hash_network.c hash.c
OBJS = $(SOURCES:%c=%o)

all : $(EXEC)

#################### LINKING

tlv : $(OBJS)
	$(CC) $^ -o tlv $(CRYPTO) $(GLIBLINK) -std=gnu99

test_neighbour_exe : test_neighbour.o neighbour.o
	$(CC) $(CFLAGS) test_neighbour.o neighbour.o $(GLIBLINK) -o test_neighbour_exe

test_tlv_exe : test_tlv_manager.o tlv_manager.o hash.o
	$(CC) $(CFLAGS) test_tlv_manager.o tlv_manager.o hash.o -o test_tlv_exe $(CRYPTO)

test_data_exe : test_data_manager.c data_manager.o hash.o
	$(CC) $(CFLAGS) test_data_manager.c data_manager.o hash.o -o test_data_exe $(PKGCONFIG) $(CRYPTO)

###################### OBJECT FILES

dazibao.o : dazibao.c tlv_manager.h neighbour.h data_manager.h hash.h inondation.h peer_state.h maintain_neighbours.h
	$(CC) $(CFLAGS) -c $< -o $@  $(GLIBHEADER) -std=gnu99

inondation.o : inondation.c inondation.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)

maintain_neighbours.o : maintain_neighbours.c maintain_neighbours.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)

tlv_manager.o : tlv_manager.c tlv_manager.h hash.h peer_state.h
	$(CC) $(CFLAGS) -c $< -o $@  $(GLIBHEADER) -std=gnu99

data_manager.o : data_manager.c data_manager.h hash.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)-std=gnu99

neighbour.o : neighbour.c neighbour.h
	$(CC) $(CFLAGS) -c $< -o $@ -std=gnu99

hash_network.o : hash_network.c hash_network.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)

hash.o : hash.c hash.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)-std=gnu99

test_neighbour.o : test_neighbour.c neighbour.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)

test_tlv_manager.o : test_tlv_manager.c tlv_manager.h
	$(CC) $(CFLAGS) -c $< -o $@ $(GLIBHEADER)

###################### CLEAN

clean:
	rm -rf *.o

cleanall: clean
	rm -rf *~ $(EXEC)
