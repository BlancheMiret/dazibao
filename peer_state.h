#ifndef __PEER_STATE_H__
#define __PEER_STATE_H__

#include "neighbour.h"
#include <glib.h>
#include <glib/gprintf.h>

struct pstate_t {
	uint64_t            node_id;
	uint16_t            num_seq; // network order
	char                data[192]; 
	char                node_hash[16]; 
	char                network_hash[16]; 
	struct neighbour    neighbour_table[15];
	GHashTable          *data_table;
};

#endif