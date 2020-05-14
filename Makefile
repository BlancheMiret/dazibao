CC = gcc
CFLAGS = -Wall -g -std=gnu99 `pkg-config --cflags glib-2.0` 
LDFLAGS = -lcrypto `pkg-config --libs glib-2.0`
SRC = dazibao.c inondation.c maintain_neighbours.c tlv_manager.c data_manager.c neighbour.c hash_network.c hash.c
OBJS = $(SRC:%c=%o)
EXEC = dazibao

all : $(EXEC)

#LINKING

dazibao : $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

#COMPILATION

%.o:%.c
	@echo " building $(@:%.o=%) . . . "
	@$(CC) -c $< -o $@ $(CFLAGS)

dazibao.o : dazibao.c inondation.h maintain_neighbours.h tlv_manager.h data_manager.h neighbour.h hash.h peer_state.h 
inondation.o : inondation.c inondation.h tlv_manager.h data_manager.h neighbour.h hash_network.h hash.h peer_state.h
maintain_neighbours.o : maintain_neighbours.c maintain_neighbours.h tlv_manager.h peer_state.h
tlv_manager.o : tlv_manager.c tlv_manager.h hash.h
data_manager.o : data_manager.c data_manager.h hash.h
neighbour.o : neighbour.c neighbour.h
hash_network.o : hash_network.c hash_network.h data_manager.h hash.h
hash.o : hash.c hash.h

#CLEANING

clean:
	rm -rf *.o

cleanall: clean
	rm -rf *~ $(EXEC)
