CC = gcc
CFLAGS = -Wall -g -std=gnu99 `pkg-config --cflags glib-2.0` 
LDFLAGS = -lcrypto `pkg-config --libs glib-2.0`
SOURCES = dazibao.c inondation.c maintain_neighbours.c tlv_manager.c data_manager.c neighbour.c hash_network.c hash.c
OBJS = $(SOURCES:%c=%o)
EXEC = tlv

all : $(EXEC)

#################### LINKING

tlv : $(OBJS)
	$(CC) $^ -o tlv $(LDFLAGS)

###################### OBJECT FILES

#%.o:%.c
#	$(CC) -c $< -o $@ $(CFLAGS)

dazibao.o : dazibao.c inondation.h maintain_neighbours.h tlv_manager.h data_manager.h neighbour.h hash_network.h hash.h peer_state.h 
	$(CC) -c $< -o $@ $(CFLAGS) 

inondation.o : inondation.c inondation.h tlv_manager.h data_manager.h neighbour.h hash_network.h peer_state.h
	$(CC) -c $< -o $@ $(CFLAGS)

maintain_neighbours.o : maintain_neighbours.c maintain_neighbours.h tlv_manager.h peer_state.h
	$(CC) -c $< -o $@ $(CFLAGS) 

tlv_manager.o : tlv_manager.c tlv_manager.h hash.h
	$(CC) -c $< -o $@ $(CFLAGS)

data_manager.o : data_manager.c data_manager.h hash.h
	$(CC) -c $< -o $@ $(CFLAGS)

neighbour.o : neighbour.c neighbour.h
	$(CC) -c $< -o $@ $(CFLAGS)

hash_network.o : hash_network.c hash_network.h data_manager.h hash.h
	$(CC) -c $< -o $@ $(CFLAGS)

hash.o : hash.c hash.h
	$(CC) -c $< -o $@ $(CFLAGS)

###################### CLEAN

clean:
	rm -rf *.o

cleanall: clean
	rm -rf *~ $(EXEC)
