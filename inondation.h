#ifndef __INONDATION_H__
#define __INONDATION_H__

#include "data_manager.h" 
#include "neighbour.h"
#include "peer_state.h" 
#include "tlv_manager.h" 

/* Répond à chaque TLV contenu dans dtg selon le protocole d'inondation */
void respond_to_dtg (struct dtg_t *dtg, int socket, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state);

#endif