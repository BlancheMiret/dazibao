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
#include <glib.h>
#include <glib/gprintf.h>
//#include "tlv.h"
#include "hash.h"
#include "data_manager.h"
#include "neighbour.h" // <--- ATTENTION NOUVEAU MODULE DES VOISINS
#include "tlv_manager.h"
#include "inondation.h"
#include "peer_state.h"
#include "maintain_neighbours.h"

#define SIZE 1024

char *data;
uint16_t new_sequence;
uint64_t node_id;

void print_hexa(char hash[16]) {
	for (int i = 0; i < 16; i++) {
		printf("%02x", hash[i]);
	}
	printf("\n");
}

GHashTable *table_voisins;

//variable globale pour notifier la capture d'un signal
volatile sig_atomic_t print_flag = false;


//Gestionnaire de signal
void handle_alarm(int sig) {
	print_flag = true;
}




struct pstate_t * peer_state_init(){


	struct pstate_t *peer_state = malloc(sizeof(struct pstate_t));
	memset(peer_state, 0, sizeof(struct pstate_t));

	// DATA ET NUMÉRO DE SÉQUENCE 
	data = "J'ai passé une excellente soirée mais ce n'était pas celle-ci.";
	memcpy(peer_state->data, data, strlen(data));
	peer_state->num_seq = htons(0x3E0D); // 0x3D = 61 --- 0x3E08 = 15880 
	//0x3E0D = 15885

	// -- ID DE NOTRE NOEUD -- 

	//Asmaa: Il faut qu'on change le type du noeud en unsigned char ou convertir uint64_t avant d'envoyer car c'est un entier
	peer_state->node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
	//printf("node_id %" PRIu64"\n", peer_state->node_id) ;

	//peer_state->node_id=htobe64(node_id);d

	peer_state->data_table = create_data_table();
	add_data(peer_state->data_table, peer_state->node_id, peer_state->num_seq, peer_state->data);

	return peer_state;


}




/************ Ajout du premier voisin permanent ************/ 

//Gérer le cas où on arrive pas à trouver/ajouter un voisin permanent

//Ajouter un voisin permanent
//Initialiser la socker
int initialization(char * argv[],struct pstate_t * peer_state){

	/******** paramètres réseaux ********/

	char *dest_host = argv[1];
	char *dest_port = argv[2];
    int sockfd;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_V4MAPPED | AI_ALL;
	hints.ai_protocol = 0;

	struct addrinfo *dest_info;
	int status;
	status = getaddrinfo(dest_host, dest_port, &hints, &dest_info);

	if (status != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Initialisation de la socket
	struct addrinfo *ap;
	

	 /* IPv4 */
	char ipv4[INET_ADDRSTRLEN];
	struct sockaddr_in *addr4;

	/* IPv6 */
	char ipv6[INET6_ADDRSTRLEN];
	struct sockaddr_in6 *addr6;


	for (ap = dest_info; ap != NULL; ap = ap->ai_next) {
		sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);

		if (ap->ai_addr->sa_family == AF_INET) {
			addr4 = (struct sockaddr_in *) ap->ai_addr;
			inet_ntop(AF_INET, &addr4->sin_addr, ipv4, INET_ADDRSTRLEN);
			printf("IP du premier voisin permanent ---> %s\n", ipv4);
			printf("*************************\n");		

    		//On ajoute au départ l'adresse IPV4 du voisin permanent
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)addr4, 1);
		}

		if (ap->ai_addr->sa_family == AF_INET6) {
			addr6 = (struct sockaddr_in6 *) ap->ai_addr;
			inet_ntop(AF_INET6, &addr6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			printf("IP du premier voisin permanent ---> %s\n", ipv6);
			printf("*************************\n");

    		//On ajoute au départ l'adresse IPV6 du voisin permanent
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)addr6, 1);
		}

		if (sockfd != -1) break;
    }

	if (ap == NULL) {
		fprintf(stderr, "La connexion a échoué. \n");
		exit(1);
	}

	freeaddrinfo(dest_info);

	return sockfd;

}

int socket_parameters(int sockfd){

	//int sockfd;
	int rc;

    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	// On lie la socket au port 8080
    struct sockaddr_in6 peer;
    memset (&peer, 0, sizeof(peer));
    peer.sin6_family = AF_INET6;
    peer.sin6_port = htons(8080);


    //Paramétrage de la socket
	int one = 1;
	int size_one = sizeof one;
	// Évite le temps mort
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, size_one) < 0) {
		perror("setsockopt SO_TIMESTAMP");
		exit(2);
	}
	
	if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &one, size_one) < 0) {
		perror("setsockopt - IPV6_V6ONLY");
		exit(2);

	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, &one, size_one) < 0) {
		perror("setsockopt SO_TIMESTAMP");
		exit(2);
	}

	if (bind(sockfd, (struct sockaddr*)&peer, sizeof(peer)) < 0 ) { 
       	perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 


	/* Parametrage pour que la socket soit en mode non bloquant */

	rc = fcntl(sockfd, F_GETFL);
	if(rc < 0) {
		perror("fcntls - get"); 
		return -1;
	}
	rc = fcntl(sockfd, F_SETFL, rc | O_NONBLOCK);
	if(rc < 0) {
		perror("fcntls - set"); 
		return -1;
	}

    return 1;

}


/* --------------- INONDATION ---------------------*/

void event_loop(struct pstate_t * peer_state, int sockfd){

	
    //SIGALRM: ce signal survient lorsqu’une alarme définie par la fonction alarm(..) a expiré
	signal( SIGALRM, handle_alarm ); 
	//Alarme qui se déclenche 
	alarm(20);

	int rc;

	while(1){

		//Vérifier chaque 20 secondes si un voisin transitoire n'a pas émis de paquet depuis 70s
		if (print_flag) {


			printf("D:297 --- TIMEOUT !! (20 secondes) --- \n");
			
			sweep_neighbour_table(peer_state->neighbour_table);

			send_network_hash(sockfd, peer_state);

			//display_neighbour_table(peer_state->neighbour_table);
			printf("D:302 - Sweeptable, il reste %d voisins.\n", get_nb_neighbour(peer_state->neighbour_table));

			//ENVOI D'UN TLV NEIGHBOUR REQUEST
			//Si la table contient au moins de 5 voisins,on envoie d'un TLV neighbour request à un voisin tiré au hasard 
			
			if(get_nb_neighbour(peer_state->neighbour_table)< 5 && get_nb_neighbour(peer_state->neighbour_table) > 0 ){

				 send_neighbour_req(sockfd, peer_state);

			}

			print_flag = false;
			alarm(20);
		}

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		struct sockaddr_in6 from;
		memset(&from, 0, sizeof(from));
		socklen_t from_len = sizeof(from);
		char recvMsg[SIZE];
		memset(recvMsg, '\0', SIZE);

	

		//timeout = 20 secondes 
		int to = 50;
		struct timeval timeout = {to,0};

		int sel = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

		if(sel < 0) {
			//interrompu par un signal
			if (errno == EINTR) {   
				continue;
			}
			perror("Select failed");
			/* Handle the error properly. */
			exit(1);
		}


		//Si une réponse est arrivée sel = 1
		if (sel > 0) {

			if(FD_ISSET(sockfd,&readfds)){
				rc = recvfrom(sockfd, recvMsg, SIZE, 0, (struct sockaddr*)&from, &from_len);

				if(rc < 0) {
					if(errno == EAGAIN) {
						//return 1;
					} 
					else {
						perror("recvfrom : ");
					}
				}

				else {
					printf("***************************************************\n");
					printf("D:361 ------- Message Reçu ! --------\n");
				}

				//On vérifie si l'entête est incorrecte
				if(check_datagram_header(recvMsg) == 1) {

					char IP[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&from)->sin6_addr), IP, INET6_ADDRSTRLEN);
					printf("D:373 : - Le message provient de l'IP : %s\n", IP);

					struct dtg_t *dtg = unpack_dtg(recvMsg, rc);
					//print_dtg(dtg);
					print_dtg_short(dtg);
					printf("***************************************************\n");

					respond_to_dtg(dtg, sockfd, &from, from_len, peer_state); // <---- INONDATION 
                    
                    

                    //(A MODIFIER! PLUS TARD)
                    struct sockaddr_storage permanent_neighbour = peer_state->neighbour_table[0].socket_addr;
					maintain_neighbour_table(peer_state, from,permanent_neighbour);


				}
			}
		}

		if(sel == 0 ) {
		//Timeout pour le recvfrom, faire un goto?
		}
	}


}

int main(int argc, char * argv[]) {


	int rc;
	 
    // ----- initialisation données -----

	struct pstate_t * peer_state = peer_state_init();

    char node_hash[16];
	hash_node(peer_state->node_id, peer_state->num_seq, peer_state->data, node_hash);
	//print_hash(node_hash);


	// ----------------------------------


    // ----- initialisation de la socket et ajout du voisin permanent -----
    int sockfd = initialization(argv,peer_state);
	socket_parameters(sockfd);

    // ----- Construction d'un datagram -- 

	struct tlv_t *node_state = new_node_state(peer_state->node_id, peer_state->num_seq, node_hash, peer_state->data);
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, node_state);

    // ----- Premier TLV à envoyer au voisin permanent -----

    struct sockaddr_storage permanent_neighbour = peer_state->neighbour_table[0].socket_addr;
	
	//Envoi du paquet Node State
	rc = sendto(sockfd, datagram, datagram_length, 0, (const struct sockaddr*)&permanent_neighbour, sizeof(struct sockaddr_in6));
	
	if (rc == -1) {
		perror("sendto() error");
		exit(2);
	}

	else{

		printf("Node state envoyé!!! \n");
	}
 	
 	// -- Partie maintenance de la table de voisins & inondation -- 
 	event_loop(peer_state,sockfd);

	
	return 0;
}
