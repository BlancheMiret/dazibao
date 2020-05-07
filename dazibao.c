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
#include "new_neighbour.h" // <--- ATTENTION NOUVEAU MODULE DES VOISINS
#include "tlv_manager.h"
#include "inondation.h"
#include "peer_state.h"


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


//Asmaa: mettre cette fonction dans innondation.c ????

//Fonction qui envoie un TLV neighbour request à un voisin tiré au hasard
void send_neighbour_req(int socket, struct pstate_t * peer_state){

	struct neighbour * neighbour_choosen = pick_neighbour(peer_state->neighbour_table);
	
	char IP2[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&neighbour_choosen->socket_addr)->sin6_addr), IP2, INET6_ADDRSTRLEN);

	struct tlv_t *neighbour_req = new_neighbour_request();
	int datagram_length1;
	char *datagram1 = build_tlvs_to_char(&datagram_length1, 1, neighbour_req);
	int status = sendto(socket, datagram1, datagram_length1, 0, (const struct sockaddr*)&neighbour_choosen->socket_addr, sizeof(struct sockaddr_in6));
	if (status == -1) {
		perror("sendto() error");
		//exit(2);
	}
	else {
		printf("D:75  - TLV Neighbour Request Envoyé à l'adresse IP : %s\n", IP2);
	}

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

//Fonction qui vérifie les conditions d'ajout d'un voisin de la partie 4.2 avant de l'ajouter
//Changer le nom peut-être ?
void maintain_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from, struct sockaddr_in6 *permanent_neighbour){

	int rc;
	//Cas où l'émetteur n'est pas présent et la table contient au moins 15 entrées
	if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 &&  get_nb_neighbour(peer_state->neighbour_table) == 15){
		printf("D:95  - Impossible d'ajouter le voisin (déjà 15 ?)\n");
	}
	printf("A\n");

	//Si on a moins de 15 voisins et find_neighbour renvoie -1 alors le voisin n'existe pas et il faut l'ajouter
	if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 &&  get_nb_neighbour(peer_state->neighbour_table) < 15){
		printf("B\n");

		//ajout d'un voisin permanent
		//Remarque : on l'a déjà ajouté au début (ligne 283), donc cette condition est inutile

		/**if(compare_addr(&permanent_neighbour->sin6_addr, &from.sin6_addr) == 0){
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 1);
			printf("D:105 - Voisin permanent ajouté!! \n");
		}**/

		//ajout d'un voisin transitoire
		//On compare l'adresse du from avec l'adresse du voisin permanent, 
		//si elles sont différentes on ajoute le voisin transitoire
		if(compare_addr(&permanent_neighbour->sin6_addr, &from.sin6_addr) != 0){
			
			rc=add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 0);
			if(rc == 0)
			{
				printf("D:111 - voisin transitoire ajouté!! \n");
				char IP[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &(from.sin6_addr), IP, INET6_ADDRSTRLEN);
				printf("D:118 - L'adresse IP du voisin transitoire ajouté est : %s\n", IP);
			}
		}

		//Affichage de la table de voisins :
		display_neighbour_table(peer_state->neighbour_table);
		printf("D:122 - Nombre voisins dans table des voisins : %d\n", get_nb_neighbour(peer_state->neighbour_table));
	}

	//Si le voisin est déjà présent mettre à jour la date de dernière réception de paquet
	if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) != -1 ){

		//A VERIFIER
		update_last_reception(peer_state->neighbour_table, (struct sockaddr_storage*)&from);
	}

}

//Fonction pour l'envoi d'un TLV network hash à tous les voisins chaque 20s 
//la mettre dans innondation.c ?
void send_network_hash(int socket, struct pstate_t * peer_state){

	struct tlv_t *network_hash=new_network_hash(peer_state->network_hash) ;
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, network_hash);

    //Parcourir la table de voisin et envoyer à chaque voisin

	int count = 0;
	int status;
	for (int i = 0; i < NBMAX; i++) {
		if (peer_state->neighbour_table[i].exists) {
			count += 1;
			status = sendto(socket, datagram, datagram_length, 0, (const struct sockaddr*)&peer_state->neighbour_table[i].socket_addr, sizeof(struct sockaddr_in6));
			if (status == -1) {
				perror("sendto() error");
				//exit(2);
			}
			else {
				printf("D:163  - TLV Network Hash envoyé");
			}
		}
	}
	
	
}


int main (int argc, char * argv[]) {

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

	//Asmaa: Il faut qu'on change le type du noeud en unsigned char ou convertir uint64_t avant d'envoyer car c'est un entier
	peer_state->node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
	//printf("node_id %" PRIu64"\n", peer_state->node_id) ;

	char node_hash[16];
	hash_node(peer_state->node_id, peer_state->num_seq, peer_state->data, node_hash);
	//print_hash(node_hash);

	peer_state->data_table = create_data_table();
	add_data(peer_state->data_table, peer_state->node_id, peer_state->num_seq, peer_state->data);

	// ----------------------------------


	// -- CONSTRUCTION D'UN DATAGRAM -- 
	struct tlv_t *node_state = new_node_state(peer_state->node_id, peer_state->num_seq, node_hash, peer_state->data);
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, node_state);


	/******** paramètres réseaux ********/

	char *dest_host = argv[1];
	char *dest_port = argv[2];

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

	// On lie la socket au port 8080
    struct sockaddr_in6 peer;
    memset (&peer, 0, sizeof(peer));
    peer.sin6_family = PF_INET6;
    peer.sin6_port = htons(8080);

	for (ap = dest_info; ap != NULL; ap = ap->ai_next) {
		sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);

		if (ap->ai_addr->sa_family == AF_INET) {
			addr4 = (struct sockaddr_in *) ap->ai_addr;
			inet_ntop(AF_INET, &addr4->sin_addr, ipv4, INET_ADDRSTRLEN);
			printf("IP du premier voisin permanent ---> %s\n", ipv4);
			printf("*************************\n");

			if (bind(sockfd, (struct sockaddr*)&peer, sizeof(peer)) < 0 ) { 
       			perror("bind failed"); 
        		exit(EXIT_FAILURE); 
    		} 

    		//On ajoute au départ l'adresse IPV4 du voisin permanent
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)addr4, 1);
		}

		if (ap->ai_addr->sa_family == AF_INET6) {
			addr6 = (struct sockaddr_in6 *) ap->ai_addr;
			inet_ntop(AF_INET6, &addr6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			printf("IP du premier voisin permanent ---> %s\n", ipv6);
			printf("*************************\n");

			if (bind(sockfd, (struct sockaddr*)&peer, sizeof(peer)) < 0 ) { 
       			perror("bind failed"); 
        		exit(EXIT_FAILURE); 
    		} 

    		//On ajoute au départ l'adresse IPV6 du voisin permanent
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)addr6, 1);
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


			



			printf("D:297 --- TIMOUT !! (20 secondes) --- \n");
			//A VERIFIER
			sweep_neighbour_table(peer_state->neighbour_table);

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
                    

					maintain_neighbour_table(peer_state, from, addr6);


				}
			}
		}

		if(sel == 0 ) {
		//Timeout pour le recvfrom, faire un goto?
		}
	}

	return 0;
}
