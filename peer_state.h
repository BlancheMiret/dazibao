#ifndef __PEER_STATE_H__
#define __PEER_STATE_H__

#include "neighbour.h"
#include <glib.h>
#include <glib/gprintf.h>

struct pstate_t {
	uint64_t            node_id; // <-- quand même besoin de savoir quel est notre propre noeud au moment de la réponse à un TLV Node State
	uint16_t            num_seq; // <-- stocké en format réseau |
	char                data[192]; //                          | --> en fait, pas nécessaire dans le node_state, mais à ajouter dans la table des données
	char                node_hash[16]; // <--- ne bouge pas     | 
	char                network_hash[16]; // <---- à mettre à jour quand nécessaire
	struct neighbour    neighbour_table[15];
	GHashTable          *data_table; // <---- hash des noeuds à mettre à jour quand nécessaire
};

#endif