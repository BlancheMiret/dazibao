#include "tlv_manager.h"


// ****************************************************************************
// --------------------------- NVLLES FONCTIONS -------------------------------

/*
Fonctions
- create pour chaque type de tlv, prend paramètres, renvoit un pointeur vers un struct tlv 
(- un fonction build tlv -> char )
- une fonction build_dtg qui prend tlv + tlv + tlv... -> dtg -> char 

- une fonction char -> dtg qui renvoit un pointeur vers un dtg tout frais et liste chainée de struct tlv
--> ensuite switch sur tlv->type (ps, les tlv reçus peuvent se traiter indépendamment les uns des autres )
*/

// ----------------------------------------------------------------------------
// -------------------------- FONCTIONS CREATION TLV --------------------------

// Fonction interne
void *new_tlv() {
	struct tlv_t *tlv = malloc(sizeof(struct tlv_t));
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

/*

	struct in6_addr {
      	unsigned char s6_addr[16];
	};

  	struct sockaddr_in6 {
      	sa_family_t sin6_family;
      	in_port_t sin6_port;
      	struct in6_addr sin6_addr;
	};

*/

void *new_neighbour_request(){
	struct tlv_t *tlv = new_tlv();
	tlv->type = 2;
	tlv->length = 0;
	return tlv;
}

/* port en format RÉSEAU DANS LES PARAMÈTRES*/
void *new_neighbour(struct in6_addr IP, in_port_t port) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 3;
	tlv->length = 18; 

	struct neighbour_b *neighbour_body = malloc(sizeof(struct neighbour_b));
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

/* seqno en format RÉSEAU DANS LES PARAMÈTRES*/
void *new_node_hash(uint64_t node_id, uint16_t seqno, char nodehash[16]) {
	struct tlv_t *tlv = new_tlv();
	tlv->type = 6;
	tlv->length = 26;

	struct nodehash_b *nodehash_body = malloc(sizeof(struct nodehash_b));
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
	memset(warning_body, 0, sizeof(struct warning_b));
	memcpy(warning_body->message, message, strlen(message));

	tlv->body.warning_body = warning_body;
	return tlv;
}

// ----------------------------------------------------------------------------
// ----------------------- FONCTION CREATION DATAGRAMME -----------------------

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
			memcpy(addr+2, tlv->body.warning_body->message, strlen(tlv->body.warning_body->message) - 1);
			break;

		default :
			printf("Error type of TLV in write_tlv\n");
			exit(1);
	}

	addr = addr + tlv->length + TLV_HEADER;

	return addr;
}


void *build_dtg(int *size_dtg, int nbtlv, ...) { // <----- free les tlv au passage par exemple ? 
	va_list valist;
	*size_dtg = 0; //sans compter le header
	struct tlv_t *tlv;
	va_start(valist, nbtlv);

	// --- RÉCUPÉRER LA TAILLE NÉCESSAIRE ET L'ÉCRIRE À L'ADRESSE size_dtg;

	for(int i = 0; i < nbtlv; i++) {
		tlv = va_arg(valist, struct tlv_t *);
		*size_dtg += tlv->length + TLV_HEADER;
	}

	*size_dtg += DTG_HEADER;
	printf("Total size should be : %d\n", *size_dtg);

	// ---- CRÉER CHAINE DE CARACTÈRE ET INITIALISER LE HEADER

	char *dtg = malloc(*size_dtg);
	memset(dtg, 0, *size_dtg);
	uint8_t magic = 0x5F;
    uint8_t ver = 0x1;
    memcpy(dtg, &magic, 1);
    memcpy(dtg+1, &ver, 1);

    // ---- ÉCRIRE CONTENU DES TLV UN PAR UN

    char *begin_tlv = dtg + DTG_HEADER;
    va_start(valist, nbtlv);

    for(int i = 0; i < nbtlv; i++) {
    	tlv = va_arg(valist, struct tlv_t *);
    	begin_tlv = write_tlv(tlv, begin_tlv);
    }
	return dtg; 
}

// ----------------------------------------------------------------------------
// --------------------------- FONCTION D'AFFICHAGHE --------------------------

// #include <arpa/inet.h>

//const char *inet_ntop(int af, const void *src,
//                      char *dst, socklen_t cnt);
//AF_INET6
//src doit pointer sur une structure struct in6_addr 
//(octets stockés dans l'ordre du réseau) qui est convertie dans la représentation 
//la plus appropriée de l'adresse IPv6. 
//Le tampon dst doit mesurer au minimum INET6_ADDRSTRLEN octets.

void print_IP_addr(struct in6_addr *sin6_addr) {
	char buf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, sin6_addr, buf, INET6_ADDRSTRLEN); // <---- DONC ICI CONSIDÉRÉ FORMAT RÉSEAU DANS LA STRUCT
	printf("IP address is : %s\n", buf);
}

void print_hash(char hash[16]) { // <---- code de Dao je crois, à revoir...
	for (int i = 0; i < 16; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}


void print_node_id(uint64_t node_id) {
	printf("Node id is : %"PRIu64"\n", node_id);
}

void print_seqno(uint16_t seqno) { // <---- stocké en format host dans le tlv 
	printf("Seq num is : %"PRIu16, ntohs(seqno)); // <------------- ICI CONSIDÉRÉ FORMAT RÉSEAU DANS LA STRUCT 
	printf("\n");
}

void print_tlv(struct tlv_t *tlv) {
	printf(" -------------- TLV --------------\n");
	printf("TLV type : %d\n", tlv->type);
	printf("TLV length : %d\n", tlv->length);

	switch(tlv->type) {
		case 0:
			printf("This is a Pad1, nothing to display.\n");
			break;

		case 1:
			printf("This is a PadN, nothing to display.\n");
			break;

		case 2:
			printf("This is a Neighbour request, nothing to display.\n");
			break;

		case 3:
			printf("This is a Neighbour tlv.\n");
			print_IP_addr(&(tlv->body.neighbour_body->iPv6_addr));
			printf("Port is : %d\n", ntohs(tlv->body.neighbour_body->port)); // <----- ICI CONSIDÉRÉ FORMAT RÉSEAU DANS LA STRUCT
			break;

		case 4:
			printf("This is a Network Hash.\n");
			printf("Network hash is : ");
			print_hash(tlv->body.nethash_body->network_hash);
			break;
			
		case 5:
			printf("This is a Network State Request, nothing to display.\n");
			break;

		case 6:
			printf("This is a Node Hash.\n");
			print_node_id(tlv->body.nodehash_body->node_id);
			print_seqno(tlv->body.nodestate_body->seq_no);
			printf("Node hash is : ");
			print_hash(tlv->body.nodehash_body->node_hash);
			break;

		case 7:
			printf("This is a Node State Request.\n");
			print_node_id(tlv->body.nodestatereq_body->node_id);
			break;

		case 8:
			printf("This is a Node State.\n");
			print_node_id(tlv->body.nodestate_body->node_id);
			print_seqno(tlv->body.nodestate_body->seq_no);
			printf("Node Hash is : ");
			print_hash(tlv->body.nodestate_body->node_hash);
			printf("Data is : %s\n", tlv->body.nodestate_body->data);
			break;

		case 9:
			printf("This is a Warning.\n");
			printf("Message is : %s\n", tlv->body.warning_body->message);
			break;

		default :
			printf("This is not a TLV.\n");
			exit(1);

	}
}




