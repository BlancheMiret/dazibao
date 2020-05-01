#include "hash_network.h"


int hash_network(GHashTable *data_table, char *final_hash) {
	int nb_data = g_hash_table_size(data_table);
	int CONCATSIZE = nb_data * 16; // <-- 16 = taille d'un node_hash
	char hash_concat[CONCATSIZE];

	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, data_table);
	int i = 0;
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		memcpy(hash_concat + i, ((struct data_t*)value)->node_hash, 16);
		i++;
    }

	hash(hash_concat, CONCATSIZE, final_hash);
	return 0;
}