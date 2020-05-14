#ifndef __MAINTAIN_NEIGHBOURS_H__
#define __MAINTAIN_NEIGHBOURS_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "peer_state.h"

/* Compare deux adresses ipv6, si ils sont égaux retourne 0 */
int compare_addr(struct in6_addr *IP1, struct in6_addr *IP2);

/* Vérifie les conditions d'ajout d'un voisin de la partie 4.2 avant de l'ajouter */
void update_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from);

/* Envoie un TLV neighbour request à un voisin tiré au hasard */
void send_neighbour_req(int socket, struct pstate_t * peer_state);

/* Pour l'envoi d'un TLV network hash à tous les voisins chaque 20s */
int send_network_hash(int socket, struct pstate_t * peer_state);

#endif

