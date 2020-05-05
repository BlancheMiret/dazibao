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
#include "tlv.h"
#include "hash.h"
#include "data_manager.h"
#include "new_neighbour.h" // <--- ATTENTION NOUVEAU MODULE DES VOISINS
#include "tlv_manager.h"
#include "inondation.h"


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


//Compare deux adresses ipv6, si ils sont égaux retourne 0
int compare_addr(struct in6_addr *IP1, struct in6_addr *IP2) {
	int i = 0;
	for(i = 0; i < 16; ++i) {
		if (IP1->s6_addr[i] < IP2->s6_addr[i]) return -1;
		else if (IP1->s6_addr[i] > IP2->s6_addr[i])  return 1;
	}
	return 0;
}


int main (void) {

	//SIGALRM: ce signal survient lorsqu’une alarme définie par la fonction alarm(..) a expiré
	signal( SIGALRM, handle_alarm ); 
	//Alarme qui se déclenche 
	alarm(20); 

	// ----- INITIALISATION DONNÉES -----

	struct pstate_t *peer_state = malloc(sizeof(struct pstate_t));
	memset(peer_state, 0, sizeof(struct pstate_t));
	// DATA ET NUMÉRO DE SÉQUENCE 
	data = "J'ai passé une excellente soirée mais ce n'était pas celle-ci.";
	memcpy(peer_state->data, data, strlen(data));
	peer_state->num_seq = htons(0x3E08); // 0x3D = 61 --- 0x3E08 = 15880 

	// -- ID DE NOTRE NOEUD -- 
	peer_state->node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
	printf("node_id %" PRIu64"\n", peer_state->node_id) ;

	char node_hash[16];
	hash_node(peer_state->node_id, peer_state->num_seq, peer_state->data, node_hash);
	print_hash(node_hash);

	peer_state->data_table = create_data_table();
	add_data(peer_state->data_table, peer_state->node_id, peer_state->num_seq, peer_state->data);

	// ----------------------------------


	// -- CONSTRUCTION D'UN DATAGRAM -- 
	struct tlv_t *node_state = new_node_state(peer_state->node_id, peer_state->num_seq, node_hash, peer_state->data);
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, node_state);


	/******** paramètres réseaux ********/

	char *dest_host = "jch.irif.fr";
	char *dest_port = "1212";

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
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
			printf("IP de jch.irif.fr ---> %s\n", ipv4);
			printf("*************************\n");
		}
		if (ap->ai_addr->sa_family == AF_INET6) {
			addr6 = (struct sockaddr_in6 *) ap->ai_addr;
			inet_ntop(AF_INET6, &addr6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			printf("IP de jch.irif.fr ---> %s\n", ipv6);
			printf("*************************\n");
		}

		if (sockfd != -1) break;
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


	/* Parametrage pour que la socket soit en mode non bloquant */
	int rc;
	rc = fcntl(sockfd, F_GETFL);
	if(rc < 0) {perror("fcntls - get"); return -1;}
	rc = fcntl(sockfd, F_SETFL, rc | O_NONBLOCK);
	if(rc < 0) {perror("fcntls - set"); return -1;}


	//Envoi du paquet Node State
	status = sendto(sockfd, datagram, datagram_length, 0, ap->ai_addr, ap->ai_addrlen);
	if (status == -1) {
		perror("sendto() error");
		exit(2);
	}


	/* --------------- PARTIE MAINTENANCE DE LA LISTE DE VOISINS ? (pour l'instant juste un test pour afficher les TLV reçus) ---------------------*/

	while(1){

		//Vérifier chaque 20 secondes si un voisin transitoire n'a pas émis de paquet depuis 70s
		if ( print_flag ) {

			printf("timeout !! (20 secondes) ");
			//A VERIFIER
			sweep_neighbour_table(peer_state->neighbour_table);

			//display_neighbour_table(peer_state->neighbour_table);
			printf("Nombre de voisins après exécution sweep_neighbour_table: %d\n", get_nb_neighbour(peer_state->neighbour_table));

			//Envoyer un TLV neighbour request, refaire cette partie en tirant au hasard une entrée de la table
			//Tirer au hasard un index i ?

			if(get_nb_neighbour(peer_state->neighbour_table)< 5 && get_nb_neighbour(peer_state->neighbour_table) > 0 ){

				/**
				char *datagram1 = main_datagram();
				// Création de message Node state 
				char neighbour_req[SIZE];
				//taille de node state
				int neighbour_req_len = Neighbour_request(neighbour_req);

				//taille du datagrame final qu'on va envoyer
				int datagram_length1 = set_msg_body(datagram1, neighbour_req, neighbour_req_len);
				**/

				struct neighbour * neighbour_choosen = pick_neighbour(peer_state->neighbour_table);

				struct tlv_t *neighbour_req = new_neighbour_request();
				int datagram_length1;
				char *datagram1 = build_tlvs_to_char(&datagram_length1, 1, neighbour_req);
				status = sendto(sockfd, datagram1, datagram_length1, 0, (const struct sockaddr*)&neighbour_choosen->socket_addr, sizeof(struct sockaddr_in6));
				if (status == -1) {
					perror("sendto() error");
					//exit(2);
				}
				else printf("TLV NEIGHBOUR REQUEST ENVOYE");

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

		// again:
		// printf("check here for neighbours");

		//timeout = 20 secondes 
		int to = 20;
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
						return 1;
					} 
					else {
						perror("recvfrom : ");
					}
				}

				else printf("Message Reçu !\n");

				//On vérifie si l'entête est incorrecte
				if(check_datagram_header(recvMsg) == 1) {

					/* TEST NOUVEAU MODULE
					print_datagram(recvMsg) ;*/

					struct dtg_t *dtg = unpack_dtg(recvMsg, rc);
					print_dtg(dtg);

					respond_to_dtg(dtg, sockfd, &from, from_len, peer_state); // <---- INONDATION 

					char IP[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&from)->sin6_addr), IP, INET6_ADDRSTRLEN);
					printf("THE IP ADDRESS IS : %s\n", IP);


					//Cas où l'émetteur n'est pas présent et la table contient au moins 15 entrées
					if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 &&  get_nb_neighbour(peer_state->neighbour_table) == 15){
						printf("IMPOSSIBLE D'AJOUTER");
					}
					printf("A\n");

					if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 &&  get_nb_neighbour(peer_state->neighbour_table) < 15){
						printf("B\n");
						//ajout d'un voisin permanent
						if(compare_addr(&addr6->sin6_addr, &from.sin6_addr) == 0){
							add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 1);
							printf("voisin jch.irif.fr ajouté!! \n");
						}

						//ajout d'un voisin transitoir
						else{
							add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 0);
						}

						struct neighbour *neighbour_table = peer_state->neighbour_table;

						char IP2[INET6_ADDRSTRLEN];
						inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&neighbour_table[0].socket_addr)->sin6_addr), IP2, INET6_ADDRSTRLEN);
						printf("(from neighbour table) THE IP ADDRESS IS : %s\n", IP2);

						//Affichage de la table de voisins :
						display_neighbour_table(peer_state->neighbour_table);
						printf("NOMBRES DE VOISINS : %d\n", get_nb_neighbour(peer_state->neighbour_table));
					}

					//Si le voisin est déjà présent mettre à jour la date de dernière réception de paquet
					if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) != -1 ){

						//A VERIFIER
						update_last_reception(peer_state->neighbour_table, (struct sockaddr_storage*)&from);
					}
				}
			}
		}

		if(sel == 0 ) {
		//Timeout pour le recvfrom, faire un goto?
		}
	}

	return 0;
}
