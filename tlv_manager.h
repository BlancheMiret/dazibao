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

#include "hash.h"


#define DTG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024


struct neighbour_b { // <------------ SIZE = 18
	struct in6_addr 	iPv6_addr; // <------------- STOCKÉ EN FORMAT RÉSEAU
	in_port_t 			port; //<---- STOCKÉ EN FORMAT RÉSEAU
};

struct nethash_b { // <------------ SIZE = 16
	char 		network_hash[16]; 
};

struct nodehash_b { // <------------- SIZE = 26 // (26+ TLV HEADER) * 15 = 420 + DTG HEADER =  424 --> à priori dépasse jamais taille max de 1024 d'un dtg...
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
	char      	data[192];
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
	struct tlv_t 	*next;
};

struct dtg_t {
	uint8_t 		magic;
	uint8_t 		version;
	uint16_t		body_length; // <----- STOCKÉ FORMAT RÉSEAU
	struct tlv_t 	*tlv_list;
	int 			nb_tlv;
};

// Fonctions de créations de TLV : renvoient un pointeur vers un struct tlv_t contenant les bons paramètres
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

// Fonctions d'affichage d'un tlv individuel ou d'un datagramme entier (et tous les tvl qu'il peut contenir)
void print_tlv(struct tlv_t *tlv);
void print_dtg(struct dtg_t *dtg);

// Fonction à utiliser au moment d'envoyer un datagramme :
// Prend un nombre illimité de tlv et renvoie un pointeur de datagramme tout prêt à envoyer sur le réseau
// size_dtg est mis à jour pour correspondre à la taille du datagramme
void *build_tlvs_to_char(int *size_dtg, int nbtlv, ...);

// Fonction à utiliser à la reception d'un datagramme :
// Renvoie un pointeur vers un struct dtg_t qui contient une liste chaînée de tlv.
// print_dtg() permet alors d'afficher le datagramme en entier
void *unpack_dtg(char *buf, int size_dtg);



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

*/

#endif