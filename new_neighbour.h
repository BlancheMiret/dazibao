#ifndef __NEW_NEIGHBOUR_H__
#define __NEW_NEIGHBOUR_H__

#define NBMAX 15

#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct neighbour {
		int					exists;
		int 				permanent;
		struct timeval		last_reception;
		struct sockaddr		socket_addr;
};

int get_nb_neighbour(struct neighbour *neighbour_table);
int add_neighbour(struct neighbour *neighbour_table, struct sockaddr *key, int perm);
int find_neighbour(struct neighbour *neighbour_table, struct sockaddr *key);
int update_last_reception(struct neighbour *neighbour_table, struct sockaddr *key);
int sweep_neighbour_table(struct neighbour *neighbour_table);
void display_neighbour_table(struct neighbour *neighbour_table);

#endif