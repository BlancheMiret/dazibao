#ifndef __INONDATION_H__
#define __INONDATION_H__

#include "data_manager.h"  // pour manipuler structure dtg
#include "new_neighbour.h" // pour manipuler la table des voisins (en tirer un au hasard)
#include "peer_state.h" // pour manipuler la structure
#include "tlv_manager.h" // pour créer des tlv à envoyer

void respond_to_dtg (struct dtg_t *dtg, int socket, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state);

#endif