#include <stdio.h> //perror, snprintf
#include <stdlib.h> //exit

#include <glib.h>
#include <glib/gprintf.h>

#include <sys/time.h>
#include <arpa/inet.h>

#include "neighbour.h"

/* ATTENTION : ON PART DU PRINCIPE QUE LES VALEURS SONT ALLOUÉES DYNAMIQUEMENT ?? 
--> VOIR G_HASH_TABLE_FULL, SINON FUITE DE MÉMOIRE
*/

struct neighbour {
		int 				permanent;
		struct timeval		last_reception;
};

// Renvoie un pointeur vers une nouvelle hashtable
void* create_neigh_table() {
	return g_hash_table_new(NULL, NULL);
}

// Renvoie le nombre d'éléments dans neighbour_table
int get_table_len(GHashTable *neighbour_table) { 
	return g_hash_table_size(neighbour_table);
}

// Ajoute un voisin dans "neighbour_table" associé à "key" et dont "last_reception" est initilisé au temps courant
// Retourne -1 en cas d'erreur, 0 sinon
int add_neighbour(GHashTable *neighbour_table, struct sockaddr *key, int perm) { 
	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		return -1;
	}	
	struct neighbour *value = malloc(sizeof(struct neighbour));
	memset(value, 0, sizeof(struct neighbour));
	value->permanent = perm;
	value->last_reception = tp;

	g_hash_table_insert(neighbour_table, key, value);
	return 0;
}

/*
void delete_neighbour(GHashTable *neighbour_table, struct sockaddr *key) { 
	g_hash_table_remove(neighbour_table, key);
}
*/

// Met à jour le champ "last_reception" de la valeur associée à "key" dans "neighbour_table" avec le temps courant 
// Retourne -1 en cas d'erreur, 0 sinon
int update_last_reception(GHashTable *neighbour_table, struct sockaddr *key) { 
	struct neighbour *value = g_hash_table_lookup(neighbour_table, key);
	if (value == NULL) {
		perror("Key not present in hashtable.\n");
		return -1;
	}
	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		return -1;
	}
	value->last_reception = tp;
	g_hash_table_replace(neighbour_table, key, value);
	return 0;
}

/*
int is_permanent(GHashTable *neighbour_table, struct sockaddr *key) {
	struct neighbour *value = g_hash_table_lookup(neighbour_table, key);
	return value->permanent;
}
*/

// Fonction interne : return 0 si "value" n'a pas été mis à jour depuis moins de 70 secondes et n'est pas de type permanent
int is_obsolete(void *key, void *value, void* user_data) { 
	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		exit(1);
	}
	int r = tp.tv_sec - ((struct neighbour *)value)->last_reception.tv_sec;
	//printf("Difference is : %d\n", r);
	//printf("Is permanent ? %d\n", ((struct neighbour *)value)->permanent);
	return (r > 70) && !(((struct neighbour *)value)->permanent);
}


// Supprime chaque paire clé-valeur de "neighbour_table" dont la valeur est obsolète
// Retourne le nombre d'éléments supprimés
int sweep_neighbour_table(GHashTable *neighbour_table) {
	return g_hash_table_foreach_remove(neighbour_table, is_obsolete, NULL);
}

// Fonction interne : affiche une paire clé - valeur
void display_neighbour(void *key, void *value, void *user_data) {
	struct sockaddr *k = (struct sockaddr *)key;
	struct neighbour *v = (struct neighbour *)value;
	char IP[INET6_ADDRSTRLEN];
	int family;
	int port;
	switch(k->sa_family) {
		case AF_INET:
			inet_ntop(AF_INET, (struct sockaddr_in *)k, IP, INET6_ADDRSTRLEN);
			family = 4;
			port = ((struct sockaddr_in *)k)->sin_port;
			break;
		case AF_INET6:
			inet_ntop(AF_INET6, (struct sockaddr_in6 *)k, IP, INET6_ADDRSTRLEN);
			family = 6;
			port = ((struct sockaddr_in6 *)k)->sin6_port;
			break;
	}

	printf("--------- NEW NEIGHBOUR --------- \n");
	printf("- KEY\n");
	printf("sa_family is : %d\n", family);
	printf("port is %d\n", port);
	printf("IP address is : %s\n", IP); 

	printf("- VALUE\n");
	printf("Is this neighbour permanent ? : %d\n", v->permanent);
	printf("Time of last reception : \n");
	printf("Seconds since Jan. 1, 1970 : %ld\n", v->last_reception.tv_sec);
	printf("Microseconds since Jan. 1, 1970 : %d\n", v->last_reception.tv_usec);
	printf("\n");	

}


// Affiche l'intégralité de la table "neighbour_table"
void display_neighbour_table(GHashTable *neighbour_table) {
	if(get_table_len(neighbour_table) == 0) {
		printf("The table is empty.\n");
		return;
	}
	g_hash_table_foreach(neighbour_table, display_neighbour, NULL);
}



















