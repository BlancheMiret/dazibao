#ifndef __NEW_NEIGHBOUR_H__
#define __NEW_NEIGHBOUR_H__

#define NBMAX 15

#include <sys/time.h>
#include <arpa/inet.h>

// -------------------------------- STRUCTURE ---------------------------------

struct neighbour {
		int							exists;
		int 						permanent;
		struct timeval				last_reception;
		struct sockaddr_storage		socket_addr;
};

// --------------------------------- ECRITURE ---------------------------------

/* Ajoute un voisin dans neighbour_table associé à la socket key et dont "last_reception" est initilisé au temps courant */
int add_neighbour(struct neighbour *neighbour_table, struct sockaddr_storage *key, int perm);

/* Met à jour à l'heure courante le champ last_reception du neighbour de socket_addr key */
int update_last_reception(struct neighbour *neighbour_table, struct sockaddr_storage *key);

/* Supprime chaque voisin de neighbour_table dont la dernière réception remonte à plus de 70 secondes */
int sweep_neighbour_table(struct neighbour *neighbour_table);

// --------------------------------- LECTURE ----------------------------------

/* Renvoie l'indice dans neighbour_table du voisin de socket_addr key, -1 si le voisin n'est pas trouvé */
int find_neighbour(struct neighbour *neighbour_table, struct sockaddr_storage *key);

/* Renvoie l'adresse d'un voisin choisi au hasard dans neighbour_table */
struct neighbour* pick_neighbour(struct neighbour *neighbour_table);

/* Renvoie le nombre de voisins dans neighbour_table */
int get_nb_neighbour(struct neighbour *neighbour_table);

// -------------------------------- AFFICHAGE ---------------------------------

void display_neighbour(struct neighbour *n);
void display_neighbour_table(struct neighbour *neighbour_table);

#endif
