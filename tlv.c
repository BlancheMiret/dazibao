#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/select.h>
#include <net/if.h>
#include <locale.h>
#include <openssl/sha.h>
#include "tlv.h"

#define MSG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024

/*
REMARQUES GÉNÉRALE :
- Attention à ne pas écrire à des adresses de pointeurs 
dont on ignore la taille allouée en mémoire
- Rappel : un datagramme peut contenir toute une série de TLV différents, 
Les TLV ne "mesureront" jamais 1000 octets ! 
(218 max je crois, taille des tlv les plus gros : Node State)
- Globalement, il serait logique pour moi de renvoyer des pointeurs pour chaque fonction
Et d'allouer la taille mémoire de ses pointeurs DANS la fonction. 
*/

/*
PROPOSITIONS MODIFICATION : 
- Sur toutes les fonctions de création de tlv : 
ne pas passer la chaine de caractère en paramètre ! 
Voir détails dans les commentaires des fonctions
- Harmoniser les noms des paramètres
(ex : dtg pour datagramme)
*/

/*
PROPOSITIONS FONCTIONS À ÉCRIRE ?
- 
*/



//Initialise le datagramme msg avec les valeurs de magic et version
// NB : ne pourrait-on pas n'avoir aucun paramètre et renvoyer directement
// nn pointeur vers un nouveau datagramme donc magic et version seraient initialisés ?
int main_datagram(char * msg){
    memset(msg, 0, SIZE);
	uint8_t magic = 0x5F;
	uint8_t ver = 0x1;
	memcpy(msg, &magic, 1);
	memcpy(msg+1, &ver, 1);
	return MSG_HEADER;
}


/****** longueur du message principal *******/


//A modifier ???

// Prend un datagramme et retourne la longeur de son body
// NB : pas besoin d'utiliser deux variables différentes ?
uint16_t get_body_length(char * message)
{
    uint16_t length;
    memcpy(&length, message + 2, 2);
    uint16_t len = ntohs(length);
    return len;
}

// Prend un datagramme, un chaine de caractères body et une longeur 
// Écrit len octets de body à l'adresse message + 4
// NB : vérifier si len < 1024 - 4 ? (SIZE - 4) ou ignorer la possibilité ? 
// Sommes-nous sûres des conversions ?
int set_msg_body(char *message, char *body, uint16_t len)
{
    
    //uint16_t convert_len = ntohs(get_body_length(message));

    memcpy(message + 4 , body, len);

    //convert_len = len;

    //from host-byte order to network-byte order 
    uint16_t convert_h = htons(len);
    memcpy(message + 2, &convert_h, 2);

    return MSG_HEADER + len;
}



//TLV  (revoir ces fonctions et l'utilisation de ntohs)

// Prend un tlv et retourne son type
// NB : conversion ? 
uint8_t getTLV_TYPE(char * tlv) {
	uint8_t type;
	memcpy(&type, tlv, 1);
	return type;
}

// Prend un tlv et retourne sa longueur
// NB : conversion ?
uint8_t getTLV_LENGTH(char * tlv) {
	uint8_t len;
	memcpy(&len, tlv+1, 1);
	return len;
}

// Prend un tlv et retourne l'adresse de son body
char * getTLV_BODY(char * tlv) {
	return tlv+TLV_HEADER;
}

//Création Pad1

// Prend une chaine de caractère, 
// NB : Même chose que pour main_datagram et toutes les fonctions suivantes
// Est-il bien nécessaire d'avoir la chaine de caractère en paramètre ? 
// Ne serait-il pas plus pratique de la créer dans la fonction directement, avec malloc ?
// (Surtout que, attention dangereux : tu mets à 0 1000 octets à partir de pad
// sans avoir aucune idée de la taille allouée à pad.)
// Et la fonction retournerait l'adresse de pad ?

// D'ailleurs, pour le tlv PAD1, sa longueur est 1 octet !!
// Rappel : un datagramme peut contenir plusieurs TLV
int Pad1(char * pad) {
	memset(pad, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x0;
	memcpy(pad, &type, 1);
	return 1;
}


//Création de PadN
// NB Mêmes commentaires que précédemment
int PadN(char * pad, uint8_t len) {
	memset(pad, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x1;
	memcpy(pad, &type, 1);
	memcpy(pad+1, &len, 1);
	return TLV_HEADER+len;
}


//Creation de Neighbour request


int Neighbour_request(char * neighbourReq) {
	memset(neighbourReq, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x2;
	uint8_t len = 0x0;
	memcpy(neighbourReq, &type, 1);
	memcpy(neighbourReq+1, &len, 1);
	return TLV_HEADER;
}



//Creation de Neighbour 


int Neighbour(char * neigbour, struct in6_addr IP, in_port_t port) {
	memset(neigbour, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x3;
	//len = 18
	uint8_t len = 0x12;
	memcpy(neigbour, &type, 1);
	memcpy(neigbour+1, &len, 1);
	memcpy(neigbour+2, &IP, 16);
	memcpy(neigbour+18, &port, 2);
	return len+TLV_HEADER;
}



//Creation de Network Hash  


int Network_hash(char * network_hash, char * hash){
    memset(network_hash, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x4;

 	uint8_t len = 0x0;
	memcpy(network_hash, &type, 1);
	memcpy(network_hash+1, &len, 1);
	return len+TLV_HEADER;


}

//Creation de Network State Request


int Network_state_request(char * network_req){
    memset(network_req, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x5;

 	uint8_t len = 0x0;
	memcpy(network_req, &type, 1);
	memcpy(network_req+1, &len, 1);
	return len+TLV_HEADER;


}

//Creation de Node Hash

int Node_hash(char * node_hash, uint64_t node_id, uint16_t seqno, char * hash){
    memset(node_hash, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x6;
    //len = 26
 	uint8_t len = 0x1A;
	memcpy(node_hash, &type, 1);
	memcpy(node_hash+1, &len, 1);
	memcpy(node_hash+2, &node_id, 8);
	memcpy(node_hash+10, &seqno, 2);
	memcpy(node_hash+12, &hash, 16);
	return len+TLV_HEADER;


}

//Creation de Node State Request


int Node_state_request(char * node_state_req, uint64_t node_id ){

    memset(node_state_req, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x7;
    //len = 26
 	uint8_t len = 0x8;
	memcpy(node_state_req, &type, 1);
	memcpy(node_state_req+1, &len, 1);
	memcpy(node_state_req+2, &node_id, 8);

	return len+TLV_HEADER;

}


//Creation de Node State
// NB : pourquoi pas uint16_t pour node_hash ? 
// Ça fonctionne une addition hexadecimale + size_t (unsigned int) dans un uint8_t...?
int Node_state(char * nodestate, uint64_t node_id, uint16_t seqno, char * node_hash,  char * data, size_t data_length) {
	memset(nodestate, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x8;
 	uint8_t len = 0x1A + data_length;
 	//uint64_t new_node_id = htonll(node_id);
	memcpy(nodestate, &type, 1);
	memcpy(nodestate+1, &len, 1);
	memcpy(nodestate+2, &node_id, 8);
	memcpy(nodestate+10, &seqno, 2);
	memcpy(nodestate+12, node_hash, 16);
	memcpy(nodestate+28, data, strlen(data));
	return len+TLV_HEADER;
}


//Creation de warning

int Warning(char * warning, char * message, int message_length) {
	memset(warning, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x9;
	uint8_t len = message_length;
	memcpy(warning, &type, 1);
	memcpy(warning+1, &len, 1);
	memcpy(warning+2, &message, message_length);
	return len+TLV_HEADER;
}



// ****************************************************************************

/*
int main (void) {

   //ID DE NOTRE NOEUD
	uint64_t node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
    printf("node_id %llu\n" PRIu64, node_id) ;
    printf("\n");
    printf("node_id %llu\n", node_id);

    // NUMÉRO DE SÉQUENCE DE LA DATA : en format gros-boutiste (cf page2)
    uint16_t numero_sequence = 0x2E; //46
    uint16_t new_sequence = htons(numero_sequence);

    // DATA DE NOTRE NOEUD
    char *data= "If you can talk with crowds and keep your virtue.";

    

    //OK POUR SUPPRESSION ?

    //char id[9], sequence[1]; // NB : Séquence fait deux octets ... ? // finalement je ne les ai pas utilisé, c'est à supprimer
    // NB : pourquoi ces conversions, et pas une écriture directe avec memcpy comme avant ?
    
    //sprintf( id, "%d", (int)node_id);
    //sprintf( sequence, "%d", numero_sequence);
    //

    //concaténation de node_id & numero de sequence & data pour le hash
    char triplet[100]; 
    memcpy(triplet, &node_id, 8);
    memcpy(triplet+8, &numero_sequence, 2);
    memcpy(triplet+10, &data, strlen(data)+1);
   


    //On convertit pour pouvoir l'utiliser dans la méthode SHA256
    //const char * new_triplet = (const char *)triplet;
    //unsigned char *res = SHA256(new_triplet, strlen(new_triplet), 0);
	unsigned char *res = SHA256((const unsigned char*)triplet, strlen(triplet), 0);
    //On tronque le résultat pour avoir 16 octets (pas sure que ca soit correcte) (Si ça me paraît bien)
    char node_hash[100];
    memcpy(node_hash, &res, 16);
	

    //Creation de message global à envoyer
    char *datagram = malloc(SIZE*sizeof(char));
    main_datagram(datagram);

    // Création de message Node state 
    char nodestate[SIZE];
 
    //taille de node state
    int node_state_len = Node_state(nodestate,node_id, new_sequence, node_hash, data,strlen(data));
    
    //taille du datagrame final qu'on va envoyer
    int datagram_length = set_msg_body(datagram, nodestate, node_state_len);


// ****** paramètres réseaux *******

    char *dest_host = "jch.irif.fr";
    char *dest_port = "1212";

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *dest_info;
    int status;

    if ((status = getaddrinfo(dest_host, dest_port, &hints, &dest_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        exit(2);
    }

    // Initialisation de la socket
    struct addrinfo *ap;
    int sockfd;
    for (ap = dest_info; ap != NULL; ap = ap->ai_next)
    {
        sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
        if (sockfd != -1)
            break;
    }

    if (ap == NULL)
    {
        fprintf(stderr, "socket() error\n");
        freeaddrinfo(dest_info);
        exit(2);
    }

    //Param?rage de la socket
    int one = 1;
    int size_one = sizeof one;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, size_one) < 0) {
        perror("setsockopt SO_TIMESTAMP");
        exit(2);
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, &one, size_one) < 0) {
        perror("setsockopt SO_TIMESTAMP");
        exit(2);
    }

    //Parametrage pour que la scket soit en mode non bloquant 
    // cree une erreur :
    //recvmsg: Resource temporarily unavailable
    //Message recu de :
    //Segmentation fault (core dumped)
    
    // status = fcntl(sockfd, F_GETFL);
    // if(status < 0) {}
    // status = fcntl(sockfd, F_SETFL, status | O_NONBLOCK);
    // if(status < 0) {}
  
    //Envoi du paquet Node State
   status = sendto(sockfd, datagram, datagram_length, 0, ap->ai_addr, ap->ai_addrlen);
    if (status == -1)
    {
        perror("sendto() error");
        exit(2);
    }

	return 0;
}
*/
