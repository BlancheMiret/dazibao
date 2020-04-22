#define MSG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024

/*
struct in6_addr {
      unsigned char s6_addr[16];
};
*/



struct neighbour_b { // <------------ SIZE = 18
	struct in6_addr 	iPv6_addr;
	uint16_t 			port; //<---- ici en réseau ou host ??
}

struct nethash_b { // <------------ SIZE = 16
	char 		network_hash[16]; 
}

struct nodehash_b { // <------------- SIZE = 26
	uint64_t	node_id;
	uint16_t	seq_no;
	char 		node_hash[16];
}

struct nodestatereq_b {
	uint64_t	node_id;
}

struct nodestate_b { // <---------- SIZE = 26 + taille DATA
	uint64_t  	node_id;
	uint16_t  	seq_no;
	char      	node_hash[16];
	char      	data[192];
};

union tlv_body {
	struct neighbour_b		neighbour_body;
	struct nethash_b		nethash_body;
	struct nodehash_b		nodehash_body;
	struct nodestatereq_b	nodestatereq_body;
	struct nodestate_b 		nodestate_body;
	struct warning_b		warning_body;
};
  
struct tlv_t {
	int 			type; // <------- ENUM À CONSTRUIRE ?
	int 			length; //pour Pad1, Neighbour Request, Network State Request : 0 / pour PadN : différent de zéro !
	union tlv_body 	*body; //pour Pad1, PadN, Neighbour Request, Network State Request à NULL (pas de body)
};

struct dtg_t {
	int 			magic;
	int 			version;
	uint16_t		body_length;
	struct tlv_t 	*tlv_list;
	int 			nb_tlv;
}

/*
To do :
- objet structure passé de fonction en fonction 

Fonctions niveau supérieur <---- À COMPLÉTER
- La donnée est-elle la même que chez nous ?
- Le node hash est-il le même que le notre ? 
- Ne pas oublier de mettre à jour les node hash, notre node hash, etc.

Fonctions
- create pour chaque type de tlv, qui renvoit un pointeur vers un tlv
(- un fonction build tlv -> char )
- une fonction build tlv + tlv + tlv... -> dtg -> char 

- une fonction char -> dtg qui renvoit un pointeur vers un dtg
--> ensuite switch sur tlv->type (ps, les tlv reçus peuvent se traiter indépendamment les uns des autres )

*/



void* main_datagram();
uint16_t get_body_length(char * message);
int check_datagram_header(char * msg_received);
int set_msg_body(char *message, char *body, uint16_t len);
uint8_t get_tlv_type(char * tlv);
uint8_t get_tlv_length(char * tlv);
char * get_tlv_body(char * tlv);
int Pad1(char * pad);
int PadN(char * pad, uint8_t len);
int Neighbour_request(char * neighbourReq);
int Neighbour(char * neigbour, struct in6_addr IP, in_port_t port);
int Network_hash(char * network_hash, char * hash);
int Network_state_request(char * network_req);
int Node_hash(char * node_hash, uint64_t node_id, uint16_t seqno, char * hash);
int Node_state_request(char * node_state_req, uint64_t node_id );
int Node_state(char * nodestate, uint64_t node_id, uint16_t seqno, char * node_hash,  char * data, size_t data_length);
int Warning(char * warning, char * message, int message_length);
void print_datagram(char *msg);

