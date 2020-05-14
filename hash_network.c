#include "hash_network.h"

/* Prend un table trié contenant déjà nb_el éléments et introduit val en conservant le tableau trié*/
void insert(uint64_t tab[], size_t nb_el, uint64_t *val) {
	int i;
	for(i = nb_el; (i > 0) && (tab[i-1] > *val); i--) {
		tab[i] = tab[i-1];
	}
	tab[i] = *val;
}


/* Calcule le hash du réseau connu par data_table et l'écrit à l'adresse final_hash*/
int hash_network(GHashTable *data_table, char *final_hash) {
	int nb_data = g_hash_table_size(data_table);

	uint64_t id_tab[nb_data]; 
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, data_table);
	int i = 0;
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		insert(id_tab, i, key);
		i++;
    }

    int CONCATSIZE = nb_data * 16;
	char hash_concat[CONCATSIZE];
    for(i = 0; i < nb_data; i++) {
    	value = g_hash_table_lookup(data_table, &id_tab[i]);
    	memcpy(hash_concat + i*16, ((struct data_t*)value)->node_hash, 16);
    }

	hash(hash_concat, CONCATSIZE, final_hash);
	return 0;
}
