#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include <glib.h>
#include <glib/gprintf.h>

// -------------------------------- STRUCTURE ---------------------------------

// KEY = uint64_t node_id

// VALUE : structure des valeurs utilisées dans la hashtable
struct data_t {
	uint16_t	seq_no; // network order
	char		data[192]; 
	char		node_hash[16];
};


// --------------------------------- CREATION ---------------------------------

/* Renvoie un pointeur vers une nouvelle GHashTable */
void *create_data_table();

// --------------------------------- ECRITURE ---------------------------------

/* Ajoute un élément à data_table, de clé node_id, de valeur struct data_t contenant les champ seq_no, data, et le hash_node est calculé */
int add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]);

/* Met à jour le champ seq_no de la valeur associée à self_id dans data_table par new_seqno */
int update_self_seq_num(GHashTable *data_table, uint64_t self_id, uint16_t new_seqno);

// ---------------------------------- LECTURE ---------------------------------

/* Renvoie le nombre d'éléments dans data_table */
int get_data_table_len(GHashTable *data_table);

/* Renvoie le numéro de séquence, en ordre réseau, de la valeur associée à la clé node_id dans data_table*/
uint16_t get_seq_no(GHashTable *data_table, uint64_t node_id);

/* Renvoie 1 si seq_no1 >> seq_no2, 0 sinon */
int is_greater_than(uint16_t seq_no1, uint16_t seq_no2);

/* Renvoie 1 si la valeur existe, 0 sinon */
int data_exists(GHashTable *data_table, uint64_t node_id);

// -------------------------------- AFFICHAGE ---------------------------------

void print_data(GHashTable *data_table, uint64_t node_id);
void display_data_table(GHashTable *data_table);

#endif
