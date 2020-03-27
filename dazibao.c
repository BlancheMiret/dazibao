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

#define SIZE 1024

char *data;
uint16_t new_sequence;
uint64_t node_id;

int main (void) {

	// DATA ET NUMÉRO DE SÉQUENCE 
	data = "If you can walk with kings nor lose the common touch";
	new_sequence = htons(0x34); //52


    // ID DE NOTRE NOEUD -- 
	// PS: Par rapport à l'affichage sur l'interface en ligne :
	// Il semble que les envois de ma machine ou de la tienne ne donnent pas le même numéro ! Le mien est a741f13ad9ac2a0c, le tien 6745c62369987348
	// Sur mon terminal, pour la vraie valeur du node_id générée, j'ai 876703126473752999, toi aussi ? 
	node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
    printf("node_id %" PRIu64, node_id) ;
    printf("\n");


    /* OK POUR SUPPRESSION ?
    char id[9], sequence[1]; // NB : Séquence fait deux octets ... ? // finalement je ne les ai pas utilisé, c'est à supprimer
    // NB : pourquoi ces conversions, et pas une écriture directe avec memcpy comme avant ?
    sprintf( id, "%d", (int)node_id);
    sprintf( sequence, "%d", numero_sequence);
    */


    //concaténation de node_id & numero de sequence & data pour le hash + 
    char triplet[100]; 
    memcpy(triplet, &node_id, 8);
    memcpy(triplet+8, &new_sequence, 2); //selon le sujet page 2, si doit bien être en big-endian dans le hash
    memcpy(triplet+10, &data, strlen(data)+1);
    //On convertit pour pouvoir l'utiliser dans la méthode SHA256
	unsigned char *res = SHA256((const unsigned char*)triplet, strlen(triplet), 0);
    //On tronque le résultat pour avoir 16 octets (pas sure que ca soit correcte) (Si ça me paraît bien)
    char node_hash[16];
    memcpy(node_hash, &res, 16);
	

	// Blanche : je refais mes remarques, mieux vaut faire les mallocs à l'intérieur des fonctions main_datagrame et Node_state et de renvoyer un pointeur !
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

    if ((status = getaddrinfo(dest_host, dest_port, &hints, &dest_info)) != 0) {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        exit(2);
    }

    // Initialisation de la socket
    struct addrinfo *ap;
    int sockfd;
    for (ap = dest_info; ap != NULL; ap = ap->ai_next) {
        sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
        if (sockfd != -1)
            break;
    }

    if (ap == NULL) {
        fprintf(stderr, "socket() error\n");
        freeaddrinfo(dest_info);
        exit(2);
    }


    //Paramétrage de la socket
    int one = 1;
    int size_one = sizeof one;
    // Évite le temps mort
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, size_one) < 0) {
        perror("setsockopt SO_TIMESTAMP");
        exit(2);
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, &one, size_one) < 0) {
        perror("setsockopt SO_TIMESTAMP");
        exit(2);
    }


    /*
    // Pour mettre la socket en polymorphe  --> erreur à l'exécution
    one = 0;
    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one)) < 0) {
    	perror("setsockopt IPV6_V6ONLY");
    	exit(2);
    }
    */

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
