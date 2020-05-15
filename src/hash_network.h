#ifndef __HASH_NETWORK_H__
#define __HASH_NETWORK_H__

#include <glib.h>
#include <glib/gprintf.h>

/* Calcule le hash du réseau connu par data_table et l'écrit à l'adresse final_hash */
int hash_network(GHashTable *data_table, char *final_hash);

#endif 