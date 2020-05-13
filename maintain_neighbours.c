#include "maintain_neighbours.h"



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
void maintain_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from, struct sockaddr_storage permanent_neighbour){

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
		//On compare l'adresse du from avec l'adresse ou les adresses du voisin permanent (A AJOUTER) 
		//si elles sont différentes on ajoute le voisin transitoire
		if(compare_addr(&(((struct sockaddr_in6 *)&permanent_neighbour)->sin6_addr), &from.sin6_addr) != 0){
			
			rc=add_neighbour(peer_state->neighbour_table, (struct sockaddr_storage*)&from, 0);
			if(rc == 0)
			{
				printf("D:111 - voisin transitoire ajouté!! \n");

				char IP1[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&permanent_neighbour)->sin6_addr), IP1, INET6_ADDRSTRLEN);
				printf("IP DANS MAINTAIN: %s\n", IP1);

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