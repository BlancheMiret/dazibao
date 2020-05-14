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

// ------------------------------- STRUCTURES ---------------------------------

// size = 18
struct neighbour_b { 
	struct in6_addr 	iPv6_addr; // network order
	in_port_t 			port; // network order
};

// size = 16
struct nethash_b { 
	char 		network_hash[16]; 
};

// size = 26
struct nodehash_b { 
	uint64_t	node_id;
	uint16_t	seq_no; // network order
	char 		node_hash[16];
};

// size = 8
struct nodestatereq_b { 
	uint64_t	node_id;
};

// size = 26 + strlen(data)
struct nodestate_b { 
	uint64_t  	node_id;
	uint16_t  	seq_no; // network order
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
	uint8_t 		type;
	uint8_t 		length; 
	union tlv_body 	body; 
	struct tlv_t 	*next;
};

struct dtg_t {
	uint8_t 		magic;
	uint8_t 		version;
	uint16_t		body_length; // network order
	struct tlv_t 	*tlv_list;
	int 			nb_tlv;
};

/* Vérifie l'entête d'un paquet udp reçu */
int check_datagram_header(char *dtg);

// ------------------------------- CREATION TLV -------------------------------

/* Renvoient un pointeur vers un struct tlv_t initialiées avec les paramètres donnés */
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

// --------------------------- CREATION DATAGRAMME ----------------------------

void *build_tlvs_to_char(int *size_dtg, int nbtlv, ...);
/* Prend une liste chainée tlv et construit un datagramme prêt à envoyer sur le réseau. */
void *build_tlvs_to_char2(int *size_dtg, int nbtlv, struct tlv_t *tlv_list);

// ---------------------------- UNPACK DATAGRAMME -----------------------------

/* Prend un paquet reçu sur le réseau et constuit une struct dtgt_t contenant la liste chaînée de TLV du paquet */
void *unpack_dtg(char *buf, int size_dtg);

// -------------------------------- AFFICHAGHE --------------------------------

void print_tlv(struct tlv_t *tlv, int short_version);
void print_dtg(struct dtg_t *dtg, int short_version);
//void print_tlv_short(struct tlv_t *tlv);
void print_dtg_short(struct dtg_t *dtg);
void print_IP_addr(struct in6_addr *sin6_addr);

#endif
