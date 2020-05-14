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
#include <stdint.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/select.h>
#include <net/if.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "inondation.h"
#include "maintain_neighbours.h"
#include "tlv_manager.h"
#include "data_manager.h"
#include "neighbour.h" 
#include "hash.h"
#include "peer_state.h"

#define SIZE 1024

//IMPORTANT: revoir la partie gestion d'erreurs et le timeout de select


int DEBUG = 0;

//variable globale pour notifier la capture d'un signal
volatile sig_atomic_t alarm_val = false;



void handle_alarm(int sig); 

struct pstate_t * peer_state_init();

int initialization(char * argv[],struct pstate_t * peer_state);

int socket_parameters(int sockfd);

void event_loop(struct pstate_t * peer_state, int sockfd);


int main(int argc, char * argv[]) {



	if(argc == 1 || argc == 2){
       
       printf("Veuillez préciser l'adresse ou nom du serveur ainsi que le numéro de port\n");
       printf("Exemple: ./dazibao jch.irif.fr 1212\n" );
       exit(1);

	}

	if(argc == 4 && strcmp (argv[3],"debug") == 0){
       DEBUG =1;
       //printf("DEBUG value= %d\n", debug );
   }


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

		printf("Node state envoyé! \n");
	}

// -- Partie maintenance de la table de voisins & inondation -- 
	event_loop(peer_state,sockfd);


	return 0;
}




//Gestionnaire de signal
void handle_alarm(int sig) {
	alarm_val = true;
}



// ---- Initialise les données et retourne la structure pstate_t

struct pstate_t * peer_state_init(){

	if(DEBUG) printf("[DEBUG] Initialisation des données...\n");

	struct pstate_t *peer_state = malloc(sizeof(struct pstate_t));
	memset(peer_state, 0, sizeof(struct pstate_t));

	// DATA ET NUMÉRO DE SÉQUENCE 
	char *data = "J'ai passé une excellente soirée mais ce n'était pas celle-ci.";
	memcpy(peer_state->data, data, strlen(data));
	peer_state->num_seq = htons(0x3E0E); // 0x3D = 61 --- 0x3E08 = 15880 
	//0x3E0E = 15886

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



// ---- Récupère les addresses du voisin permanent et les ajoute dans la table de voisins

int initialization(char * argv[],struct pstate_t * peer_state){

/******** paramètres réseaux ********/

	char *dest_host = argv[1];
	char *dest_port = argv[2];
	int sockfd;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_V4MAPPED|AI_ALL;
	hints.ai_protocol = 0;

	struct addrinfo *dest_info;
	int status;
	status = getaddrinfo(dest_host, dest_port, &hints, &dest_info);

	if (status != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
		exit(1);
	}


	struct addrinfo *ap;



/* IPv6 */
	char ipv6[INET6_ADDRSTRLEN];
	struct sockaddr_in6 *addr6;


	for (ap = dest_info; ap != NULL; ap = ap->ai_next) {
		sockfd = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);

		if(sockfd<0){
			close(sockfd);
			continue;
		}


		if (ap->ai_addr->sa_family == AF_INET6) {
			addr6 = (struct sockaddr_in6 *) ap->ai_addr;
			inet_ntop(AF_INET6, &addr6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			printf("IP du premier voisin permanent ---> %s\n", ipv6);
			printf("*************************\n");

			//On ajoute au départ l'adresse IPV6 du voisin permanent
			add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)addr6, 1);
		}

		//if (sockfd != -1) break;
	}

	if(sockfd<0){
		perror("socket error: ");
	}

	
	if(DEBUG){
		printf("NOMBRE DE VOISINS  %d\n",get_nb_neighbour(peer_state->neighbour_table));	
     	printf("[DEBUG] ---------- AFFICHAGE DE LA TABLE DES VOISINS ----------\n");
		display_neighbour_table(peer_state->neighbour_table);
		
	}


	freeaddrinfo(dest_info);

	return sockfd;

}

// ---- Ajoute d'options et paramétrage de la socket

int socket_parameters(int sockfd){

	if(DEBUG) printf("[DEBUG] Paramétrage de la socket \n");
	//int sockfd;
	int rc;

	sockfd = socket(AF_INET6, SOCK_DGRAM, 0);

	if(sockfd<0){
		perror("socket error: ");
	}
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


/* Paramétrage pour que la socket soit en mode non bloquant */

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

// ---- Boucle principale du programme pour gérer la reception et envoi des TLVs

void event_loop(struct pstate_t * peer_state, int sockfd){


	//SIGALRM: ce signal survient lorsqu’une alarme définie par la fonction alarm(..) a expiré
	 if (signal(SIGALRM, handle_alarm) == SIG_ERR) printf("can't catch SIGALRM\n");; 
	//Alarme qui se déclenche 
	alarm(20);

	int rc;

	while(1){

		//Vérifier chaque 20 secondes si un voisin transitoire n'a pas émis de paquet depuis 70s
		if (alarm_val) {


		if(DEBUG) printf("[DEBUG] --- 20 secondes se sont écoulées ! --- \n");

			rc=sweep_neighbour_table(peer_state->neighbour_table);

			//display_neighbour_table(peer_state->neighbour_table);
			if(DEBUG){
				printf("[DEBUG] sweep_neighbour_table, il reste %d voisins.\n", get_nb_neighbour(peer_state->neighbour_table));
				printf("[DEBUG] Nombre de voisins supprimés: %d\n", rc);
			

			}

			//ENVOI D'UN TLV NETWORK HASH
			rc=send_network_hash(sockfd, peer_state);

			if(DEBUG == 1 && rc == 1) printf("[DEBUG] ---> TLV NETWORK HASH envoyé à tous les voisins!\n");

			//ENVOI D'UN TLV NEIGHBOUR REQUEST
			//Si la table contient moins de 5 voisins,on envoie d'un TLV neighbour request à un voisin tiré au hasard 
			if(get_nb_neighbour(peer_state->neighbour_table)< 5 && get_nb_neighbour(peer_state->neighbour_table) > 0 ){

				send_neighbour_req(sockfd, peer_state);

			}

			alarm_val = false;
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


		//timeout = 50 secondes 
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
					printf("------- Message Reçu ! --------\n");
				}

				//On vérifie si l'entête est incorrecte
				if(check_datagram_header(recvMsg)) {

					char IP[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&from)->sin6_addr), IP, INET6_ADDRSTRLEN);
					printf("- Le message provient de l'IP : %s\n", IP);

					//Si l'émetteur n'est pas présent et si la table de voisins contient déjà 15 entrées
					if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 && get_nb_neighbour(peer_state->neighbour_table) == 15){
						
						if(DEBUG){
							printf("La table de voisin contient 15 entrées, le paquet est ignoré!\n");
						}

					}

					if(get_nb_neighbour(peer_state->neighbour_table)< 15){
						struct dtg_t *dtg = unpack_dtg(recvMsg, rc);
						
						if(dtg == NULL) continue;
						
						//print_dtg(dtg);
						//print_dtg_short(dtg,peer_state);
						print_dtg_short(dtg);
						printf("***************************************************\n");

						respond_to_dtg(dtg, sockfd, &from, from_len, peer_state); // <---- INONDATION 

						update_neighbour_table(peer_state, from);
					}

				}
			}
		}

		if(sel == 0 ) {
			//Timeout pour le recvfrom, faire un goto?
		}
	}


}