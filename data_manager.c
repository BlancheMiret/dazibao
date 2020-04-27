#include "data_manager.h"
//#include "hash.h"

/*
struct data_t {
	uint16_t	seq_no; // <-- STOCKÉ EN ORDRE HOST, PAS RÉSEAU !!!!!
	char		data[192];
	char		node_hash[16]; // <-- pas obligé de le recalculer à chaque fois tiens...
};
*/


void *create_data_table() {
	return g_hash_table_new_full(NULL, NULL, free, free);
}

int get_data_table_len(GHashTable *data_table) { 
	return g_hash_table_size(data_table);
}

// TRAVAIL EN COURS : BESOIN DU CALCUL DE NODE HASH !!
int add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]) { // <-- ATTENTION, SEQ_NO EN ORDRE HOST !!
	// créer allocation pour node_id : key
	uint64_t *key = malloc(sizeof(uint64_t));
	mempcy(key, &node_id, 8);

	// créer allocation 
	struct data *value = malloc(sizeof(struct data_t));
	memset(value, 0, sizeof(struct data_t));
	memcpy(&value->seq_no, &seq_no, 2);
	memcpy(value->data, data, 192);
	// char node_hash[16];
	// hash_node(node_id, seq_no, data, &node_hash); // <-------------------- inscrit le hash à l'adresse de node_hash 
	// memcpy(valuer->node_hash, node_hash, 16);

	g_hash_table_inser(data_table, key, value);
}




// Fonctions :

// - void *create_data_table();

// - add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]); //<-- dès qu'on ajoute une donnée, node_state doit être mise à jour !!
//		Doit ajouter une donnée, OU mettre à jour la donnée de clé node_id si elle existait
//		Doit aussi calculer tout de suite le nouveau node_hash
//		----
// 		ATTENTION, DERRIÈRE, OBLIGATION RECALCULER NETWORK HASH !!! 

// - compare_hash(GHashTable *data_table, uint64_t node_id, char node_hash_to_compare[16]); // <-- renvoie TRUE si même hash dans table de données, sinon FALSE
//		Doit comparer deux char[16]
//		Attention, si table de données ne contient pas d'entrée de clé node_id (donc lookup renvoie null) renvoie Faux, on s'en fout de savoir qu'on l'a pas

// - uint16_t get_seq_no(GHashTable *data_table, uint64_t node_id);
// - int is_greater_than(uint16_t seq_no1, uint16_t seq_no2); // <-- renvoie TRUE si seq_no1 >> seq_no2, FALSE sinon

// - update_self_seq_num(GHashTable *data_table, uint64_t self_id)

// - data_exists(GHashTable *data_table, uint64_t node_id); //< <-- renvoie TRUE si existe, FALSE sinon 