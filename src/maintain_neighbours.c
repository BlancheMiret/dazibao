#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <inttypes.h>
#include <net/if.h>
#include "maintain_neighbours.h"
#include "tlv_manager.h"
#include "debug.h"

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
void update_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from){

	//Si on a moins de 15 voisins et find_neighbour renvoie -1 alors le voisin n'existe pas et il faut l'ajouter
	if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) == -1 &&  get_nb_neighbour(peer_state->neighbour_table) < 15) {

		add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 0);
	
		//Affichage de la table de voisins :
		if(DEBUG){

			printf("[DEBUG] Nombre voisins dans table des voisins : %d\n", get_nb_neighbour(peer_state->neighbour_table));
			printf("[DEBUG] ---------- AFFICHAGE DE LA TABLE DES VOISINS ----------\n");
			display_neighbour_table(peer_state->neighbour_table);
			
		}
	}

	//Si le voisin est déjà présent mettre à jour la date de dernière réception de paquet
	if(find_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from) != -1 ){

		update_last_reception(peer_state->neighbour_table, (struct sockaddr_storage*)&from);

	}

}


//Fonction qui envoie un TLV neighbour request à un voisin tiré au hasard
void send_neighbour_req(int socket, struct pstate_t * peer_state){

	struct neighbour * neighbour_choosen = pick_neighbour(peer_state->neighbour_table);
	
	char IP[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&neighbour_choosen->socket_addr)->sin6_addr), IP, INET6_ADDRSTRLEN);

	struct tlv_t *neighbour_req = new_neighbour_request();
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, neighbour_req);
	int status = sendto(socket, datagram, datagram_length, 0, (const struct sockaddr*)&neighbour_choosen->socket_addr, sizeof(struct sockaddr_in6));
	if (status == -1) {
		perror("sendto() error");
		//exit(2);
	}
	else printf("TLV Neighbour Request Envoyé à l'adresse IP : %s\n", IP);

	free_tlv(neighbour_req);
	free(datagram);
}


//Fonction pour l'envoi d'un TLV network hash à tous les voisins chaque 20s 
//retourne 1 si le TLV a bien a été envoyé, 0 sinon 
int send_network_hash(int socket, struct pstate_t * peer_state){

    //Création du datagramme contenant le TLV network hash
	struct tlv_t *network_hash = new_network_hash(peer_state->network_hash) ;
	int datagram_length;
	char *datagram = build_tlvs_to_char(&datagram_length, 1, network_hash);

    //Parcourir la table de voisin et envoyer à chaque voisin

	int status;

	for (int i = 0; i < NBMAX; i++) {
		if (peer_state->neighbour_table[i].exists) {
			status = sendto(socket, datagram, datagram_length, 0, (const struct sockaddr*)&peer_state->neighbour_table[i].socket_addr, sizeof(struct sockaddr_in6));
			if (status == -1) {
				perror("sendto() error ");
				return 0;
			}
			
		}
	}

	free_tlv(network_hash);
	free(datagram);

	return 1;

}

