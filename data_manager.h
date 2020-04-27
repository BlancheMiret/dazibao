#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include <stdio.h> //perror, snprintf
#include <stdlib.h> //exit
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

// définir structures dans la hashtable des données

// KEY = uint64_t qui est le node_id

// VALUE : struct data

struct data_t {
	uint16_t	seq_no; // <-- STOCKÉ EN ORDRE HOST, PAS RÉSEAU !!!!!
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



#endif