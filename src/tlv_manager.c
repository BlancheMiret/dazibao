#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <inttypes.h>

#include "hash.h"
#include "tlv_manager.h"


// ----------------------------------------------------------------------------
// -------------------------- CHECK DATAGRAME HEADER --------------------------

/* Prend un paquet udp, renvoie 1 si les 4 premiers octets ont des valeurs corrects
Vérifie si magic = 95, version = 1, et que la longeur annoncée ne dépasse pas la taille du paquet reçu */
int check_datagram_header(char *dtg) {

	uint8_t magic;
	memcpy(&magic, dtg, 1);

	uint8_t version;
	memcpy(&version, dtg +1, 1);

	uint16_t length;
	memcpy(&length, dtg + 2, 2);
	length = ntohs(length);

	return (magic == 95 && version == 1 && length + DTG_HEADER < SIZE);
}

// ----------------------------------------------------------------------------
// ------------------------------- CREATION TLV -------------------------------

void *new_tlv() {
	struct tlv_t *tlv = malloc(sizeof(struct tlv_t));
	if(tlv == NULL) {
		perror("mallc");
		exit(1);
	}
	memset(tlv, 0, sizeof(struct tlv_t));
	return tlv;
}

void *new_pad1() {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 0;
	tlv->length = 0;
	return tlv;
}

void *new_padN(int nbzeros) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 1;
	tlv->length = nbzeros;
	return tlv;
}

void *new_neighbour_request(){
	struct tlv_t *tlv = new_tlv();
	tlv->type = 2;
	tlv->length = 0;
	return tlv;
}

/* Port est en format réseau dans les paramètres */
void *new_neighbour(struct in6_addr IP, in_port_t port) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 3;
	tlv->length = 18; 

	struct neighbour_b *neighbour_body = malloc(sizeof(struct neighbour_b));
	if(neighbour_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(neighbour_body, 0, sizeof(struct neighbour_b));
	neighbour_body->iPv6_addr = IP;
	neighbour_body->port = port;

	tlv->body.neighbour_body = neighbour_body;
	return tlv;
}

void *new_network_hash(char network_hash[16]) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 4;
	tlv->length = 16;

	struct nethash_b *nethash_body = malloc(sizeof(struct nethash_b));
	if(nethash_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(nethash_body, 0, sizeof(struct nethash_b));
	memcpy(nethash_body->network_hash, network_hash, 16);

	tlv->body.nethash_body = nethash_body;
	return tlv;
}

void *new_network_state_request() {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 5;
	tlv->length = 0;
	return tlv;
}

/* Seqno est en format réseau dans les paramètres */
void *new_node_hash(uint64_t node_id, uint16_t seqno, char nodehash[16]) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 6;
	tlv->length = 26;

	struct nodehash_b *nodehash_body = malloc(sizeof(struct nodehash_b));
	if(nodehash_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(nodehash_body, 0, sizeof(struct nodehash_b));
	nodehash_body->node_id = node_id;
	nodehash_body->seq_no = seqno;
	memcpy(nodehash_body->node_hash, nodehash, 16);

	tlv->body.nodehash_body = nodehash_body;
	return tlv;
}

void *new_node_state_request(uint64_t node_id) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 7;
	tlv->length = 8;

	struct nodestatereq_b *nodestatereq_body = malloc(sizeof(struct nodestatereq_b));
	if(nodestatereq_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(nodestatereq_body, 0, sizeof(struct nodestatereq_b));
	nodestatereq_body -> node_id = node_id;

	tlv->body.nodestatereq_body = nodestatereq_body;
	return tlv;
}

/* seqno en format RÉSEAU DANS LES PARAMÈTRES*/
void *new_node_state(uint64_t node_id, uint16_t seq_no, char node_hash[16], char *data) {
	if (strlen(data) > 192) exit(1);

	struct tlv_t *tlv = new_tlv();
	tlv->type = 8;
	tlv->length = 26 + strlen(data);

	struct nodestate_b *nodestate_body = malloc(sizeof(struct nodestate_b));
	if(nodestate_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(nodestate_body, 0, sizeof(struct nodestate_b));
	nodestate_body->node_id = node_id;
	nodestate_body->seq_no = seq_no;
	memcpy(nodestate_body->node_hash, node_hash, 16);
	memcpy(nodestate_body->data, data, strlen(data));

	tlv->body.nodestate_body = nodestate_body;
	return tlv;
}

void *new_warning(char *message) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 9;
	tlv->length = strlen(message);

	struct warning_b *warning_body = malloc(sizeof(struct warning_b));
	if(warning_body == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(warning_body, 0, sizeof(struct warning_b));
	memcpy(warning_body->message, message, strlen(message));

	tlv->body.warning_body = warning_body;
	return tlv;
}

// ----------------------------------------------------------------------------
// --------------------------- CREATION DATAGRAMME ----------------------------

/* Prend un tlv et l'écrit en concaténant ses données à l'adresse addr 
Renvoie l'adresse de la fin de l'écriture du tlv */
void *write_tlv(struct tlv_t *tlv, char *addr) {
	memcpy(addr, &tlv->type, 1);
	if (tlv->type != 0) memcpy(addr + 1, &tlv->length, 1);

	switch(tlv->type) {
		case 0:
		case 2:
		case 5:
			break;

		case 1:
			memset(addr + 2, 0, tlv->length);
			break;

		case 3:
			memcpy(addr + 2, &tlv->body.neighbour_body->iPv6_addr, 16);
			memcpy(addr + 18, &tlv->body.neighbour_body->port, 2);
			break;

		case 4:
			memcpy(addr + 2, tlv->body.nethash_body->network_hash, 16);
			break;

		case 6:
			memcpy(addr + 2, &tlv->body.nodehash_body->node_id, 8);
			memcpy(addr + 10, &tlv->body.nodehash_body->seq_no, 2);
			memcpy(addr + 12, tlv->body.nodehash_body->node_hash, 16);
			break;

		case 7:
			memcpy(addr + 2, &tlv->body.nodestatereq_body->node_id, 8);
			break;

		case 8:
			memcpy(addr + 2, &tlv->body.nodestate_body->node_id, 8);
			memcpy(addr + 10, &tlv->body.nodestate_body->seq_no, 2);
			memcpy(addr + 12, tlv->body.nodestate_body->node_hash, 16);
			memcpy(addr + 28, tlv->body.nodestate_body->data, strlen(tlv->body.nodestate_body->data));
			break;

		case 9:
			memcpy(addr+2, tlv->body.warning_body->message, strlen(tlv->body.warning_body->message));
			break;

		default :
			printf("Error type of TLV in write_tlv\n");
			exit(1);
	}

	if (tlv -> type != 0) addr = addr + tlv->length + TLV_HEADER; 
	else addr = addr + 1;
	//free(tlv); // <------------------------- BESOIN DE LES SUPPRIMER À UN MOMENT OÙ UN AUTRE

	return addr;
}


void *init_dtg(size_t size_dtg) {
	char *dtg = malloc(size_dtg);
	if(dtg == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(dtg, 0, size_dtg);
	uint8_t magic = 0x5F;
    uint8_t ver = 0x1;
    uint16_t length = htons(size_dtg - DTG_HEADER);
    memcpy(dtg, &magic, 1);
    memcpy(dtg+1, &ver, 1);
    memcpy(dtg+2, &length, 2);
    return dtg;
}


void *build_tlvs_to_char(int *size_dtg, int nbtlv, ...) { 
	va_list valist;
	*size_dtg = 0; //sans compter le header
	struct tlv_t *tlv;
	va_start(valist, nbtlv);

	// --- CALCULER LA TAILLE FINALE DU DATAGRAMME ET L'ÉCRIRE À L'ADRESSE size_dtg; // <----- QUE FAIRE SI TAILLE > 1024 ??

	for(int i = 0; i < nbtlv; i++) {
		tlv = va_arg(valist, struct tlv_t *);
		if (tlv->type != 0) *size_dtg += tlv->length + TLV_HEADER;
		else *size_dtg += 1;
	}

	*size_dtg += DTG_HEADER;

	// ---- CRÉER CHAINE DE CARACTÈRE ET INITIALISER LE HEADER
	char *dtg = init_dtg(*size_dtg);

    // ---- ÉCRIRE CONTENU DES TLV UN PAR UN

    char *begin_tlv = dtg + DTG_HEADER;
    va_start(valist, nbtlv);

    for(int i = 0; i < nbtlv; i++) {
    	tlv = va_arg(valist, struct tlv_t *);
    	begin_tlv = write_tlv(tlv, begin_tlv); // <-- write_tlv modifie l'adresse pointée par begin_tlv
    }
	return dtg; 
}



/* Prend une liste chaînée de tlv et construit un datagramme prêt à envoyer sur le réseau.
Le dépassement de 1024 octets n'est pas vérifié, il est nécessaire de le vérifier avant d'utiliser la fonction. 
La taille du datagramme finale est inscrite à l'adresse size_dtg */
void *build_tlvs_to_char2(int *size_dtg, int nbtlv, struct tlv_t *tlv_list) { // <-- Cf Test dans test_tlv_manager
	struct tlv_t *tlv = tlv_list;
	*size_dtg = 0; 

	while(tlv != NULL) {
		if(tlv->type != 0) *size_dtg += tlv->length + TLV_HEADER;
		else *size_dtg += 1;
		tlv = tlv->next;
	}

	*size_dtg += DTG_HEADER;
	char *dtg = init_dtg(*size_dtg);

	char *begin_tlv = dtg + DTG_HEADER;
	tlv = tlv_list;
	while(tlv != NULL) {
		begin_tlv = write_tlv(tlv, begin_tlv);
		tlv = tlv->next;
	}
	return dtg;
}

// ----------------------------------------------------------------------------
// ---------------------------- UNPACK DATAGRAMME -----------------------------

/* Libère la mémoire de chaque TLV de la liste chainée de dtg */
void free_tlvs_of_dtg(struct dtg_t *dtg) {
	struct tlv_t *tlv = dtg->tlv_list;
	while(tlv != NULL) {
		struct tlv_t *temp = tlv;
		tlv = tlv->next;
		free(temp);
	}
}


/* Prend un pointeur vers une adresse d'un paquet reçu et construit une struct TLV.
Renvoie une erreur si la taille annoncée du TLV dépasse la taille restante du paquet.
Renvoie un pointeur vers cette structure. */
int unpack_next_tlv(char *from, struct tlv_t *tlv, size_t size_left_dtg) {

	if (size_left_dtg < TLV_HEADER) return -1;

	memcpy(&tlv->type, from, 1);
	memcpy(&tlv->length, from + 1, 1);

	if(tlv->length + TLV_HEADER > size_left_dtg) return -1;

	switch(tlv->type) {
		case 1 :
		case 2 :
		case 5 :
			break;

		case 3 :
			tlv->body.neighbour_body = malloc(sizeof(struct neighbour_b));
			if(tlv->body.neighbour_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.neighbour_body, 0, sizeof(struct neighbour_b));
			memcpy(&tlv->body.neighbour_body->iPv6_addr, from + 2, 16);
			memcpy(&tlv->body.neighbour_body->port, from + 18, 2);
			break;

		case 4 :
			tlv->body.nethash_body = malloc(sizeof(struct nethash_b));
			if(tlv->body.nethash_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.nethash_body, 0, sizeof(struct nethash_b));
			memcpy(tlv->body.nethash_body->network_hash, from + 2, 16);
			break;

		case 6 :
			tlv->body.nodehash_body = malloc(sizeof(struct nodehash_b));
			if(tlv->body.nodehash_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.nodehash_body, 0, sizeof(struct nodehash_b));
			memcpy(&tlv->body.nodehash_body->node_id, from + 2, 8);
			memcpy(&tlv->body.nodehash_body->seq_no, from + 10, 2);
			memcpy(tlv->body.nodehash_body->node_hash, from + 12, 16);
			break;

		case 7 :
			tlv->body.nodestatereq_body = malloc(sizeof(struct nodestatereq_b));
			if(tlv->body.nodestatereq_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.nodestatereq_body, 0, sizeof(struct nodestatereq_b));
			memcpy(&tlv->body.nodestatereq_body->node_id, from + 2, 8);
			break;

		case 8 : 
			tlv->body.nodestate_body = malloc(sizeof(struct nodestate_b));
			if(tlv->body.nodestate_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.nodestate_body, 0, sizeof(struct nodestate_b));
			memcpy(&tlv->body.nodestate_body->node_id, from + 2, 8);
			memcpy(&tlv->body.nodestate_body->seq_no, from + 10, 2);
			memcpy(tlv->body.nodestate_body->node_hash, from + 12, 16);
			memcpy(tlv->body.nodestate_body->data, from + 28, tlv->length - 26);
			break;

		case 9 :
			tlv->body.warning_body = malloc(sizeof(struct warning_b));
			if(tlv->body.warning_body == NULL) {
				perror("malloc");
				exit(1);
			}
			memset(tlv->body.warning_body, 0, sizeof(struct warning_b));
			memcpy(tlv->body.warning_body->message, from + 2, tlv->length);
			break;

		default :
			printf("Type TLV : %d\n", tlv->type);
			printf("Wrong type of tlv in unpack_next_tlv\n");
			return -1;
	}
	return 0;
}


/* Prend un paquet reçu sur le réseau et constuit une struct dtgt_t contenant la liste chaînée de TLV du paquet 
Renvoie un pointeur vers cette structure */
void *unpack_dtg(char *buf, int size_dtg) {

	if(size_dtg < DTG_HEADER) return NULL;

	struct dtg_t *dtg = malloc(sizeof(struct dtg_t));
	if(dtg == NULL) {
		perror("malloc");
		exit(1);
	}

	memset(dtg, 0, sizeof(struct dtg_t));
	memcpy(&dtg->magic, buf, 1);
	memcpy(&dtg->version, buf + 1, 1);
	memcpy(&dtg->body_length, buf + 2, 2);

	if(ntohs(dtg->body_length) + DTG_HEADER != size_dtg) {
		free(dtg);
		return NULL;
	}

	int decount = size_dtg - DTG_HEADER;
	buf = buf + DTG_HEADER;
	struct tlv_t **tlv = &(dtg->tlv_list);

	while(decount != 0) {
		if (buf[0] == 0) {
			buf += 1;
			decount -= 1;
			continue;
		}
		if (decount == 0) break;
		dtg->nb_tlv += 1;
		
		*tlv = malloc(sizeof(struct tlv_t));
		if(tlv == NULL) {
			perror("malloc");
			exit(1);
		}

		memset(*tlv, 0, sizeof(struct tlv_t));

		if (unpack_next_tlv(buf, *tlv, decount) < 0) {
			free_tlvs_of_dtg(dtg);
			free(dtg);
			return NULL;
		}

		decount = decount - (*tlv)->length - TLV_HEADER;
		buf = buf + (*tlv)->length + TLV_HEADER;
		tlv = &((*tlv)->next); 

	}
	return dtg;
}

// ----------------------------------------------------------------------------
// ----------------------------------- FREE -----------------------------------

/*Libère la mémoire allouée à un tlv*/
void free_tlv(struct tlv_t *tlv) {
	if (tlv == NULL) return;
	switch(tlv->type) {
		case 1 :
		case 2 :
		case 5 :
			break;

		case 3 :
			free(tlv->body.neighbour_body);
			break;

		case 4 :
			free(tlv->body.nethash_body);
			break;

		case 6 :
			free(tlv->body.nodehash_body);
			break;

		case 7 :
			free(tlv->body.nodestatereq_body);
			break;

		case 8 : 
			free(tlv->body.nodestate_body);
			break;

		case 9 :
			free(tlv->body.warning_body);
			break;

		default :
			break;
	}
	free(tlv);
}


/*Libère la mémoire de chaque noeud d'une liste chaînée de tlv. */
void free_tlv_list(struct tlv_t *tlv_list) {
	struct tlv_t *node = tlv_list;
	struct tlv_t *temp;
	while(node != NULL) {
		temp = node;
		node = node->next;
		free_tlv(temp);
	}
	tlv_list = NULL;
}


/*Libère la mémoire d'un dtg et des tlv associés */
void free_dtg(struct dtg_t *dtg) {
	free_tlv_list(dtg->tlv_list);
	free(dtg);
}

// ----------------------------------------------------------------------------
// -------------------------------- AFFICHAGHE --------------------------------


void print_IP_addr(struct in6_addr *sin6_addr) {
	char buf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, sin6_addr, buf, INET6_ADDRSTRLEN); 
	printf("IP address is : %s\n", buf);
}


void print_node_id(uint64_t node_id) {
	printf("Node id is : %"PRIu64"\n", node_id);
}


void print_seqno(uint16_t seqno) { 
	printf("Seq num is : %"PRIu16, ntohs(seqno));
	printf("\n");
}


void print_tlv_header(struct tlv_t *tlv) {
	printf("TLV type : %d\n", tlv->type);
	printf("TLV length : %d\n", tlv->length);
}


void print_tlv(struct tlv_t *tlv, int short_version) {
	if(tlv == NULL) return;

	switch(tlv->type) {
		case 0:
			if(!short_version) print_tlv_header(tlv);
			printf("This is a Pad1, nothing to display.\n");
			break;

		case 1:
			if (short_version) {
				printf("PadN\n");
				break ;
			}
			printf("	 ----------  TLV PADN  --------\n");
			print_tlv_header(tlv);
			for(int i = 0; i < tlv->length; i++) printf("0");
			printf("\n");
			break;

		case 2:
			if (short_version) {
				printf("Neigbhour Request\n");
				break;
			}
			printf("	 ---  TLV NEIGBOUR REQUEST  ---\n");
			print_tlv_header(tlv);
			break;

		case 3:
			if (short_version) {
				printf("Neighbour\n");
				break;
			}
			printf("	 -------  TLV NEIGHBOUR  ------\n");
			print_tlv_header(tlv);
			print_IP_addr(&(tlv->body.neighbour_body->iPv6_addr));
			printf("Port is : %d\n", ntohs(tlv->body.neighbour_body->port)); 
			break;

		case 4:
			if (short_version) {
				printf("Network Hash\n");
				break;
			}
			printf("	 -----  TLV NETWORK HASH  -----\n");
			print_tlv_header(tlv);
			printf("Network hash is : ");
			print_hash(tlv->body.nethash_body->network_hash);
			break;
			
		case 5:
			if (short_version) {
				printf("Network State Request\n");
				break;
			}
			printf("	 --  TLV NET. STATE REQUEST  --\n");
			print_tlv_header(tlv);
			break;

		case 6:
			if (short_version) {
				printf("Node Hash\n");
				break;
			}
			printf("	 -------  TLV NODE HASH  ------\n");
			print_tlv_header(tlv);
			print_node_id(tlv->body.nodehash_body->node_id);
			print_seqno(tlv->body.nodestate_body->seq_no);
			printf("Node hash is : ");
			print_hash(tlv->body.nodehash_body->node_hash);
			break;

		case 7:
			if (short_version) {
				printf("Node State Request\n");
				break;
			}
			printf("	 --  TLV NODE STATE REQUEST  --\n");
			print_tlv_header(tlv);
			print_node_id(tlv->body.nodestatereq_body->node_id);
			break;

		case 8:
			if (short_version) {
				printf("Node State\n");
				break;
			}
			printf("	 ------  TLV NODE STATE  ------\n");
			print_tlv_header(tlv);
			print_node_id(tlv->body.nodestate_body->node_id);
			print_seqno(tlv->body.nodestate_body->seq_no);
			printf("Node Hash is : ");
			print_hash(tlv->body.nodestate_body->node_hash);
			printf("Data is : %s\n", tlv->body.nodestate_body->data);
			break;

		case 9:
			if (short_version) {
				printf("Warning\n");
				printf("Message is : %s\n", tlv->body.warning_body->message);
				break;
			}
			printf("	 ----------  WARNING  ---------\n");
			print_tlv_header(tlv);
			printf("Message is : %s\n", tlv->body.warning_body->message);
			break;

		default :
			printf("This is not a TLV.\n");
			exit(1);

	}
}


void print_dtg_short(struct dtg_t *dtg) {
	print_dtg(dtg, 1);
}


void print_dtg(struct dtg_t *dtg, int short_version) {

	printf("Magic : %"PRIu8"\n", dtg->magic);
    printf("Version : %"PRIu8"\n", dtg->version);
    uint16_t body_length = ntohs(dtg->body_length);
    printf("Body_Length : %"PRIu16"\n", body_length);
    printf("Nb tlv in dtg : %d\n", dtg->nb_tlv);
    struct tlv_t *tlv = dtg->tlv_list;
    for(int i = 0; i < dtg->nb_tlv; i ++) {
    	if (short_version) print_tlv(tlv, 1);
    	else print_tlv(tlv, 0);
    	tlv = tlv -> next; 
    }
}
