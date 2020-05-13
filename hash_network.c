#include "hash_network.h"

// Insertion dans tableau trié
void insert(uint64_t tab[], size_t nb_el, uint64_t *val) {
	int i;
	for(i = nb_el; (i > 0) && (tab[i-1] > *val); i--) {
		tab[i] = tab[i-1];
	}
	tab[i] = *val;
}


int hash_network(GHashTable *data_table, char *final_hash) {
	int nb_data = g_hash_table_size(data_table);

	// Insérer les node_id un par un dans le tableau id_tab
	uint64_t id_tab[nb_data]; // <-- tableau qui va trier les node_id
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, data_table);
	int i = 0;
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		insert(id_tab, i, key);
		i++;
    }

    // Concaténer les hash de toutes les données par ordre croissant de node_id
    int CONCATSIZE = nb_data * 16; // <-- 16 = taille d'un node_hash
	char hash_concat[CONCATSIZE];
    for(i = 0; i < nb_data; i++) {
    	value = g_hash_table_lookup(data_table, &id_tab[i]);
    	memcpy(hash_concat + i*16, ((struct data_t*)value)->node_hash, 16);
    }

    // Calculer le hash de cette concaténation de hashs
	hash(hash_concat, CONCATSIZE, final_hash);
	return 0;
}