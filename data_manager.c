#include "data_manager.h"

#define TWOPOWSIXTEEN 65536

// ----------------------------------------------------------------------------
// --------------------------------- CREATION ---------------------------------

/* Renvoie un pointeur vers une nouvelle GHashTable. 
Des fonctions de libération de mémoire sont fournies pour gérer la mémoire en cas de suppression de donnée*/
void *create_data_table() {
	return g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
}


// ----------------------------------------------------------------------------
// --------------------------------- ECRITURE ---------------------------------

/* Ajoute un élément à data_table, de clé node_id, de valeur struct data_t contenant les champ seq_no, data, et le hash_node est calculé.
Si la clé node_id existe déjà, les champs seq_no, data et hash_node de la valeur associée sont mis à jour. */
int add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]) {
	uint64_t *key = malloc(sizeof(uint64_t));
	memcpy(key, &node_id, 8);

	struct data_t *value = malloc(sizeof(struct data_t));
	memset(value, 0, sizeof(struct data_t));
	memcpy(&value->seq_no, &seq_no, 2);
	memcpy(value->data, data, 192);
	char node_hash[16];
	hash_node(node_id, seq_no, data, node_hash); 
	memcpy(value->node_hash, node_hash, 16); // <--- ligne à réduire

	g_hash_table_insert(data_table, key, value); 
	return 0;
}


/* Met à jour le champ seq_no de la valeur associée à self_id dans data_table par new_seqno */
int update_self_seq_num(GHashTable *data_table, uint64_t self_id, uint16_t new_seqno) {
	struct data_t *value = g_hash_table_lookup(data_table, &self_id);
	if (value == NULL) return -1;
	new_seqno = ntohs(new_seqno);
	new_seqno = (new_seqno + 1)%TWOPOWSIXTEEN;
	value->seq_no = htons(new_seqno);
	return 0;
}


// ----------------------------------------------------------------------------
// ---------------------------------- LECTURE ---------------------------------

/* Renvoie le nombre d'éléments dans data_table */
int get_data_table_len(GHashTable *data_table) { 
	return g_hash_table_size(data_table);
}


/* Renvoie le numéro de séquence, en ordre réseau, de la valeur associée à la clé node_id dans data_table*/
uint16_t get_seq_no(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	uint16_t host_seq_no = ntohs(value->seq_no);
	return host_seq_no; 
}


/* Prend deux numéros de séquence, en ordre hôte, renvoie 1 si seq_no1 >> seq_no2, 0 sinon */
int is_greater_than(uint16_t seq_no1, uint16_t seq_no2) { 
	return ((seq_no1 - seq_no2)%TWOPOWSIXTEEN) < 32768;
}


/* Renvoie 1 si la valeur existe, 0 sinon */ 
int data_exists(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	return value != NULL;
} 


// ----------------------------------------------------------------------------
// -------------------------------- AFFICHAGE ---------------------------------


/* Fonction interne d'affichage */
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


/* Affiche l'intégralité de la table de données */
void display_data_table(GHashTable *data_table) {
	if(get_data_table_len(data_table) == 0) {
		printf("The table is empty.\n");
		return;
	}
	g_hash_table_foreach(data_table, display_data, NULL);
}


/* Affiche la donnée de clé node_id dans data_table*/
void print_data(GHashTable *data_table, uint64_t node_id) {
	struct data_t *value = g_hash_table_lookup(data_table, &node_id);
	if(value == NULL) return;
	display_data(&node_id, value, NULL);
}
