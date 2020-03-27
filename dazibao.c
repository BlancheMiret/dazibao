#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <stdbool.h>
//#include <unistd.h>
//#include <errno.h>
//#include <sys/types.h>
//include <sys/socket.h>
#include <netdb.h>
//#include <sys/time.h>
//#include <time.h>
//#include <stdint.h>
//#include <arpa/inet.h>
//#include <inttypes.h>
//#include <fcntl.h>
//#include <sys/select.h>
//#include <net/if.h>
//#include <locale.h>
#include <openssl/sha.h>

#include "tlv.h"

#define SIZE 1024

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
    uint16_t numero_sequence = 0x2F; //47
    uint16_t new_sequence = htons(numero_sequence);

    // DATA DE NOTRE NOEUD
    char *data= "If you can talk with crowds and keep your virtue.";

    

    /* OK POUR SUPPRESSION ?

    char id[9], sequence[1]; // NB : Séquence fait deux octets ... ? // finalement je ne les ai pas utilisé, c'est à supprimer
    // NB : pourquoi ces conversions, et pas une écriture directe avec memcpy comme avant ?
    
    sprintf( id, "%d", (int)node_id);
    sprintf( sequence, "%d", numero_sequence);
    */

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


/****** paramètres réseaux ********/

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

    /* Parametrage pour que la scket soit en mode non bloquant */
    /* cree une erreur :
    recvmsg: Resource temporarily unavailable
    Message recu de :
    Segmentation fault (core dumped)
    */
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
