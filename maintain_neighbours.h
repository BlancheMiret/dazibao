#ifndef __MAINTAIN_NEIGHBOURS_H__
#define __MAINTAIN_NEIGHBOURS_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>



#include "peer_state.h"


int compare_addr(struct in6_addr *IP1, struct in6_addr *IP2);
void update_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from);
void send_neighbour_req(int socket, struct pstate_t * peer_state);
int send_network_hash(int socket, struct pstate_t * peer_state);

#endif

