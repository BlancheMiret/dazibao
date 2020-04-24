#ifndef TLV_MANAGER_H
#define TLV_MANAGER_H 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <inttypes.h>
#include <stdarg.h>
#include <arpa/inet.h>


#define DTG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024



/*
struct in6_addr {
      unsigned char s6_addr[16];
};
*/



struct neighbour_b { // <------------ SIZE = 18
	struct in6_addr 	iPv6_addr; // <------------- STOCKÉ EN FORMAT RÉSEAU
	in_port_t 			port; //<---- STOCKÉ EN FORMAT RÉSEAU
};

struct nethash_b { // <------------ SIZE = 16
	char 		network_hash[16]; 
};

struct nodehash_b { // <------------- SIZE = 26
	uint64_t	node_id;
	uint16_t	seq_no; // <------------- STOCKÉ EN FORMAT RÉSEAU
	char 		node_hash[16];
};

struct nodestatereq_b { // <--------- SIZE = 8
	uint64_t	node_id;
};

struct nodestate_b { // <---------- SIZE = 26 + taille DATA
	uint64_t  	node_id;
	uint16_t  	seq_no; // <------------- STOCKÉ EN FORMAT RÉSEAU
	char      	node_hash[16];
	char      	data[192]; // <----------- à memset, et ainsi strlen fonctionne
};

struct warning_b {
	char		message[SIZE - DTG_HEADER - TLV_HEADER];
};

union tlv_body {
	struct neighbour_b		*neighbour_body;
	struct nethash_b		*nethash_body;
	struct nodehash_b		*nodehash_body;
	struct nodestatereq_b	*nodestatereq_body;
	struct nodestate_b 		*nodestate_body;
	struct warning_b		*warning_body;
};
  
struct tlv_t {
	uint8_t 		type; // <------- ENUM À CONSTRUIRE ?
	uint8_t 		length; //pour Pad1, Neighbour Request, Network State Request : 0 / pour PadN : différent de zéro !
	union tlv_body 	body; //pour Pad1, PadN, Neighbour Request, Network State Request à NULL (pas de body)
};

struct dtg_t {
	uint8_t 		magic;
	uint8_t 		version;
	uint16_t		body_length; // <----- STOCKÉ EN QUEL FORMAT ? RÉSEAU
	struct tlv_t 	*tlv_list;
	int 			nb_tlv;
};

void *new_pad1();
void *new_padN(int nbzeros);
void *new_neighbour_request();
void *new_neighbour(struct in6_addr IP, in_port_t port);
void *new_network_hash(char network_hash[16]);
void *new_network_state_request();
void *new_node_hash(uint64_t node_id, uint16_t seqno, char nodehash[16]);
void *new_node_state_request(uint64_t node_id);
void *new_node_state(uint64_t node_id, uint16_t seq_no, char node_hash[16], char *data);
void *new_warning(char *message);

void print_tlv(struct tlv_t *tlv);

void *build_dtg(int *size_dtg, int nbtlv, ...);



/*
To do :
- objet structure passé de fonction en fonction 
	- id-noeud
	- num sequence
	- donnée
	- HASH du noeud <--- ne bouge pas
	- HASH du réseau <---- à mettre à jour quand nécessaire 
	- table des données <----- Hash à mettre à jour quand nécessaire
	- table des voisins 

Fonctions niveau supérieur <---- À COMPLÉTER
- La donnée est-elle la même que chez nous ?
- Reception node hash : le node hash est-il le même que le notre ? 
- Ne pas oublier de mettre à jour les node hash, notre node hash, etc.

Fonctions
- create pour chaque type de tlv, prend paramètres, renvoit un pointeur vers un struct tlv 
(- un fonction build tlv -> char )
- une fonction build_dtg qui prend tlv + tlv + tlv... -> dtg -> char 

- une fonction char -> dtg qui renvoit un pointeur vers un dtg tout frais et liste chainée de struct tlv
--> ensuite switch sur tlv->type (ps, les tlv reçus peuvent se traiter indépendamment les uns des autres )

*/

#endif