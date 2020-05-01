#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include <string.h>
#include <stdio.h> //perror, snprintf
#include <stdlib.h> //exit
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "hash.h"

// KEY = uint64_t qui est le node_id

// VALUE : struct data
struct data_t {
	uint16_t	seq_no; // <-- STOCKÉ EN ORDRE RÉSEAU !!!!! ????
	char		data[192]; 
	char		node_hash[16]; // <-- pas obligé de le recalculer à chaque fois tiens...
};

// table des données n'a rien à voir avec la table des voisins

// ATTENTION, PENSER À METTRE SA PROPRE DONNÉE DANS LA TABLE DES DONNÉES AU TOUT DÉBUT DU PROGRAMME

/*
BESOINS :
- À la réception d'un Node Hash décrivant le noeud d'Id i avec le hash h : si hash sont identiques, rien à faire
		--> donc besoin de comparer les hash. 
		ATTENTION, si hash sont différents OU si Id i n'existe pas dans la table (différencier le comportement de glib ?)  répondre par un Node State Request
- À la réception d'un Node state
		- Comparer les hash, si identiques rien à faire (même fonction qu'avant) (on se demande bien pourquoi revoir)
		- Sinon
			- Si id du node state est notre propre id (DAZI) : selon rapport entre les séquences (GET ET GREATER), incrémenter notre numéro de séquence (UPDATE_SELF) ou ne rien faire // <-- dazi qui gère check node_id ??
			- Si id autre noeud
					- si pas d'entrée (ATTENTION GLIB) ou si s > s' : élimination ancienne donnée, ajout de la nouvelle (voir avec glib si il réécrit)
					- sinon rien à faire

*/

void *create_data_table();
int get_data_table_len(GHashTable *data_table);
int add_data(GHashTable *data_table, uint64_t node_id, uint16_t seq_no, char data[192]);
void print_data(GHashTable *data_table, uint64_t node_id);
int compare_hash(GHashTable *data_table, uint64_t node_id, char node_hash_to_compare[16]);
uint16_t get_seq_no(GHashTable *data_table, uint64_t node_id);
int is_greather_than(uint16_t seq_no1, uint16_t seq_no2);
int update_self_seq_num(GHashTable *data_table, uint64_t self_id);
int data_exists(GHashTable *data_table, uint64_t node_id);
void display_data_table(GHashTable *data_table);

#endif
