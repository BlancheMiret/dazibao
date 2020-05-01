#include "data_manager.h"

// KEYS : uint64_t node_id

/* VALUES : 
struct data_t {
	uint16_t	seq_no; // <-- STOCKÉ EN ORDRE PAS RÉSEAU !!!!!
	char		data[192];
	char		node_hash[16]; // <-- pas obligé de le recalculer à chaque fois tiens...
};
*/

#define TWOPOWSIXTEEN 65536


void *create_data_table() {
	return g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
}


int get_data_table_len(GHashTable *data_table) { 
	return g_hash_table_size(data_table);
}


// ATTENTION, BESOIN DE RECALCULER LE NETWORK HASH APRÈS UTILISATION DE CETTE FONCTION !!!
//		Doit ajouter une donnée, OU mettre à jour la donnée de clé node_id si elle existait
//		Doit aussi calculer tout de suite le nouveau node_hash
int add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]) { // <-- ATTENTION, SEQ_NO EN ORDRE RÉSEAU !!
	// créer allocation pour node_id : key
	uint64_t *key = malloc(sizeof(uint64_t));
	memcpy(key, &node_id, 8);

	// créer allocation 
	struct data_t *value = malloc(sizeof(struct data_t));
	memset(value, 0, sizeof(struct data_t));
	memcpy(&value->seq_no, &seq_no, 2); // <--- stocké en ordre réseau dans les struct data_t de la hashmap !!
	memcpy(value->data, data, 192);
	char node_hash[16];
	hash_node(node_id, seq_no, data, node_hash); // <-------------------- inscrit le hash à l'adresse de node_hash 
	memcpy(value->node_hash, node_hash, 16);

	g_hash_table_insert(data_table, key, value); // <--- Vérfiie que met bien à jour la valeur quand clé existe déjà... <---- !!!!!!!!!!!
	return 0;
}


void print_data(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	uint16_t host_seq_no = ntohs(value->seq_no);
	printf("--- DATA ---\n");
	printf("- Key\n");
	printf("Node id is %"PRIu64"\n", node_id);
	printf("- Value\n");
	printf("Seq numero is %"PRIu16"\n", host_seq_no);
	printf("Node hash is :");
	print_hash(value->node_hash);
	printf("Data is : %s\n", value->data);
}


// Fonction interne : affiche une paire clé - valeur
void display_data(void *key, void *value, void *user_data) {
	uint64_t *k = key;
	struct data_t *v = value;
	uint16_t host_seq_no = ntohs(v->seq_no);
	printf("--- DATA ---\n");
	printf("- Key\n");
	printf("Node id is %"PRIu64"\n", *k);
	printf("- Value\n");
	printf("Seq numero is %"PRIu16"\n", host_seq_no);
	printf("Node hash is :");
	print_hash(v->node_hash);
	printf("Data is : %s\n", v->data);

}


// Affiche l'intégralité de la table
void display_data_table(GHashTable *data_table) {
	if(get_data_table_len(data_table) == 0) {
		printf("The table is empty.\n");
		return;
	}
	g_hash_table_foreach(data_table, display_data, NULL);
}


// - compare_hash(GHashTable *data_table, uint64_t node_id, char node_hash_to_compare[16]); // <-- renvoie TRUE si même hash dans table de données, sinon FALSE
//		Doit comparer deux char[16]
//		Attention, si table de données ne contient pas d'entrée de clé node_id (donc lookup renvoie null) renvoie Faux, on s'en fout de savoir qu'on l'a pas
int compare_hash(GHashTable *data_table, uint64_t node_id, char node_hash_to_compare[16]) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	for(int i = 0; i < 16; i++) {
		if (node_hash_to_compare[i] != value->node_hash[i]) return 0;
	}
	return 1;
}


uint16_t get_seq_no(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	uint16_t host_seq_no = ntohs(value->seq_no);
	return host_seq_no; // <--- ATTENTION, STOCKÉ ORDRE RÉSEAU, RENVOYÉ EN ORDRE HOST
}


//Renvoie TRUE si seq_no1 >> seq_no2, FALSE sinon
int is_greather_than(uint16_t seq_no1, uint16_t seq_no2) { // <-- EN PARAMÈTRES: ORDRE HOST !!
	//seq_no2 << seq_no1 ??
	return ((seq_no1 - seq_no2)%TWOPOWSIXTEEN) < 32768;

}


// - update_self_seq_num(GHashTable *data_table, uint64_t self_id)
int update_self_seq_num(GHashTable *data_table, uint64_t self_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &self_id);
	uint16_t host_seq_no = ntohs(value->seq_no);
	host_seq_no = (host_seq_no + 1)%TWOPOWSIXTEEN;
	value->seq_no = htons(host_seq_no); // <-- Devrait mettre à jour la valeur directement dans la hashtable du coup, à vérifier !!!!
	//g_hash_table_insert(data_table, &self_id, value);
	return 0;
}


// Renvoie TRUE si existe, FALSE sinon 
int data_exists(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	return value != NULL;
} 

/*

void data_table_iter(GHashTable *data_table){
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, neighbour_table);
	int i=0;	
	while (g_hash_table_iter_next (&iter, &key, &value)) {
  		i++;
    	printf("indice  %d\n",i);
    }

}

*/
