#include "inondation.h"
#include "hash_network.h"

// ----------------------------------------------------------------------------
// ------------------------------ FREE TLV LIST -------------------------------

/*
Libère la mémoire de chaque noeud d'une liste chaînée de tlv.
*/
void free_tlv_list(struct tlv_t *tlv_list) {
	struct tlv_t *node = tlv_list;
	struct tlv_t *temp;
	while(node != NULL) {
		temp = node;
		node = node->next;
		free(temp);
	}
	tlv_list = NULL;
}


// ----------------------------------------------------------------------------
// ------------------------------ PRINT RESPONSE ------------------------------

/*
Prend des paramètres réseau, un nombre de tlv et une liste chaînée de tlv.
Envoie la liste chaînée de tlv.
Affiche un message de confirmation d'envoi.
*/
int send_tlv_list(int sockfd, struct sockaddr_in6 *from, size_t size_from, int nb_tlv, struct tlv_t *tlv_list) {
	int size_dtg;
	char *dtg_char = build_tlvs_to_char2(&size_dtg, nb_tlv, tlv_list);
	int rc = sendto(sockfd, dtg_char, size_dtg, 0, (struct sockaddr*)from, size_from);
	if (rc < 0){
		perror("Send to");
		return rc; // ??
	}

	// -- Message de confirmation d'envoi (on pourrait afficher le contenu, mais peut-être très très long...)

	printf("\n--- DATAGRAMME JUST SENT TO\n");
	char IP[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &from->sin6_addr, IP, INET6_ADDRSTRLEN);
	printf("IP : %s\n", IP);
	printf("Port : %d\n", ntohs(from->sin6_port));
	printf("---------------------------\n\n");

	// --

	free_tlv_list(tlv_list);
	free(dtg_char);
	return 0;
}


// ----------------------------------------------------------------------------
// ----------------------- RESPOND TO NEIGHBOUR REQUEST -----------------------

/*
Renvoie un tlv neighbour contenant les informations d'un voisin choisi au hasard dans la table des voisins.
*/
void *build_res_neighbour_req(struct pstate_t *peer_state) {
	// tire au hasard entrée table des voisins
	struct neighbour *n = pick_neighbour(peer_state->neighbour_table); // <--- fonction à modifier, renvoie int !!
	
	// Créer TLV Neighbour contenant IP et Port du Neighbour choisi 
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&n->socket_addr;
	struct tlv_t *tlv_neighbour = new_neighbour(sin6->sin6_addr,sin6->sin6_port);

	return tlv_neighbour;
}


// ----------------------------------------------------------------------------
// --------------------------- RESPOND TO NEIGHBOUR ---------------------------

/*
Renvoie un network hash contenant le network hash calculé par le pair.
*/
void *build_res_neighbour(struct pstate_t *peer_state) {
	return new_network_hash(peer_state->network_hash);
}


// ----------------------------------------------------------------------------
// ------------------------- RESPOND TO NETWORK HASH --------------------------

/*
Prend un tlv network_hash et l'état du réseau vu par le pair.
Si le hash du réseau du network hash est le même que celui calculé par le pair, rien ne se passe.
Sinon, renvoie un tlv network state request.
*/
void *build_res_network_hash(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 4) {
		printf("Shouldn't be in respond_to_network_hash function.\n");
		return NULL;
	}

	char received_hash[16];
	memcpy(received_hash, tlv->body.nethash_body->network_hash, 16);
	char known_hash[16];
	memcpy(known_hash, peer_state->network_hash, 16);

	if (compare_2_hash(received_hash, known_hash)) return NULL; // <--fonction dans module hash
	return new_network_state_request();
}


// ----------------------------------------------------------------------------
// --------------------- RESPOND TO NETWORK STATE REQUEST ---------------------

// ON SAIT QU'UN NODE HASH MESURE 28 OCTETS 
// ATTENTION À NE PAS EN METTRE TROP DANS UN DATAGRAMME !!!
// AU MAX : 1024 - 4 (DTG HEADER) = 1000 / 28 = 35 
// On ne va pas ajouter manuellement les 35 tlv dans la fonction build_char_to_tlvs...
// --> solution : nouvelle fonction build_tlvs_to_char2 qui prend une liste chainée de tlv plutôt que chaque tlv en argument

/*
Prend des paramètres réseau et l'état du réseau vu par le pair.
Envoie un node state par donnée stockée dans la table des données.
*/
int respond_to_network_state_req(int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {
	// Créer une liste chainée de tlv's (max 35 à la suite)
	struct tlv_t *tlv_list = NULL;
	struct tlv_t **pointer = &tlv_list; // <-- le pointeur qui va se "balader de next en next" sur la liste chaînée
	
	GHashTableIter iter;
	gpointer key, value;
	uint64_t node_id;
	int count = 0;

	g_hash_table_iter_init (&iter, peer_state->data_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		node_id = *((uint64_t *)key);
		*pointer = new_node_hash(node_id, ((struct data_t *)value)->seq_no, ((struct data_t *)value)->node_hash);
		pointer = &((*pointer)->next);
		count ++;

		if (count == 35) {
			pointer = NULL;
			send_tlv_list(sockfd, from, size_from, count, tlv_list);
			pointer = &tlv_list;
			count = 0;
		}
    }

    if (count != 0) {
    	pointer = NULL;
    	send_tlv_list(sockfd, from, size_from, count, tlv_list);
    }
	return 0;
}


// ----------------------------------------------------------------------------
// --------------------------- RESPOND TO NODE HASH ---------------------------

/* 
Prend un tlv node hash et l'état du réseau vu par le pair.
S'il n'a pas d'entrée pour le node_id du tlv node hash dans la table des données, 
ou que les hash sont différents : renvoie un node state request.
*/
void *build_res_node_hash(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 6) {
		printf("Shouldn't be in respond_to_ node_hash function.\n");
		return NULL;
	}
	struct data_t *value = g_hash_table_lookup(peer_state->data_table, &tlv->body.nodehash_body->node_id);
	if(value == NULL || !compare_2_hash(tlv->body.nodehash_body->node_hash, value->node_hash)) {
		return new_node_state_request(tlv->body.nodehash_body->node_id);
	}
	return NULL;

	//int rc = compare_hash(peer_state->data_table, tlv->body.nodehash_body->node_id, tlv->body.nodehash_body->node_hash); // <-- fonction à vérifier. Notamment : si pas d'entrée, renvoie bien 0 (faux) ?
	//if (rc) return NULL;
	//return new_node_state_request(tlv->body.nodehash_body->node_id);

}


// ----------------------------------------------------------------------------
// ----------------------- RESPOND TO NODE STATE REQUEST ----------------------

/*
Retourne un tlv node state.
*/
void *build_res_node_state_request(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 7) {
		printf("Shouldn't be in respond_to_node_state_request function.\n");
		return NULL;
	}

	struct data_t *value = g_hash_table_lookup(peer_state->data_table, &tlv->body.nodestatereq_body->node_id);
	if (value == NULL) {
		perror("Data does not exist.");
		return NULL;
	}

	return new_node_state(tlv->body.nodestatereq_body->node_id, value->seq_no, value->node_hash, value->data);
}


// ----------------------------------------------------------------------------
// --------------------------- RESPOND TO NODE STATE --------------------------

/*
Prend un tlv_node_state et l'état du réseau vu par le pair.
Si le hash du réseau du node_state et celui enregistré par le pair sont les mêmes, la fonction termine.
Sinon, compare les numéros de séquences, si besoin ajoute ou met à jour la donnée et recalcule le network_hash.
*/
int respond_to_node_state(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 8) {
		printf("Shouldn't be in respond_to_node_state function.\n");
		return -1;
	}

	uint64_t neigh_node_id = tlv->body.nodestate_body->node_id;
	uint16_t neigh_seq_no = tlv->body.nodestate_body->seq_no; 

	struct data_t *value = g_hash_table_lookup(peer_state->data_table, &neigh_node_id);
	//
	//int rc = compare_hash(peer_state->data_table, tlv->body.nodestate_body->node_id, tlv->body.nodestate_body->node_hash); // <-- fonction à vérifier. Notamment : si pas d'entrée, renvoie bien -1 ?
	//if (rc) return 0;

	if(value != NULL && compare_2_hash(tlv->body.nodestate_body->node_hash, value->node_hash)) return 0;

	// si iota = self id
	if (neigh_node_id == peer_state->node_id) {
		if (value == NULL) { // <--- Ne devrait pas l'être si iota = self id, mais sait-on jamais.
			perror("You forgot to add yourself to your data table.\n");
			exit(1);
		}
		update_self_seq_num(peer_state->data_table, peer_state->node_id, neigh_seq_no);
		return 0;
	}

	// iota != self id
	if(value == NULL || !(is_greater_than(ntohs(value->seq_no), ntohs(neigh_seq_no)))) { // 
		// stocker donnée (avec données node state) dans table des données
		add_data(peer_state->data_table, neigh_node_id, neigh_seq_no, tlv->body.nodestate_body->data);

		// ATTENTION LA VISION DU RÉSEAU A CHANGÉ : RECALCULER LE HASH RÉSEAU
		hash_network(peer_state->data_table, peer_state->network_hash); //<-- met à jour directement dans peer_state. Attention fonction non testée.
	}

	// Si numéro de séquence supérieur chez le pair : ne rien faire
	return 0;
}


// ----------------------------------------------------------------------------
// ------------------------------- RESPOND TO TLV -----------------------------

/*
Prend un tlv auquel il faut répondre, des paramètres réseau, l'état du réseau vu par le pair.
Selon le type de tlv, appelle la fonction necessaire.
Renvoie un pointeur vers un tlv-réponse, ou NULL (en cas d'erreur ou fonctionnement particulier de fonction - cas 5 6 8)
*/
void *respond_to_tlv(struct tlv_t *tlv, int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {
	struct tlv_t *response_tlv = NULL;
	switch(tlv->type) {
		case 0:
			// if (debug)
			printf("A TLV shouldn't have been constructed for a Pad1, God.\n");
			break;

		case 1:
			// if (debug)
			printf("Ignoring a PadN\n");
			break;

		case 2: // Neighbour request : 
			response_tlv = build_res_neighbour_req(peer_state); // <- récupère un tlv_neighbour
			break;

		case 3: // Neigbhour
			response_tlv = build_res_neighbour(peer_state); // <- récupère network hash
			break;

		case 4: // Network Hash
			response_tlv = build_res_network_hash(tlv, peer_state);  // <- récupère tlv network state request ou NULL
			break;

		case 5: // Network State Request
			respond_to_network_state_req(sockfd, from, size_from, peer_state); // <- le seul qui fait ses envois tout seul, sinon compliqué...
			break;

		case 6: // Node Hash
			response_tlv = build_res_node_hash(tlv, peer_state); // <- récupère Node State Request ou NULL
			break;

		case 7: // Node State Request
			response_tlv = build_res_node_state_request(tlv, peer_state); //<- récupère Node State ou Null (si erreur)
			break;

		case 8: //Node State
			respond_to_node_state(tlv, peer_state); // <- pas de TLV à envoyer
			break;

		case 9: //Warning
			printf("Shouldn't try to respond to Warning TLV.\n");
			break;

		default :
			break;
	}
	return response_tlv;
}


// ----------------------------------------------------------------------------
// ------------------------------- RESPOND TO DTG -----------------------------

/*
Prend une structure dtg construite par la fonction unpack_dtg, 
les paramètres réseaux nécessaires à l'envoi des réponses, 
et la structure peer_state représentant le point de vue de mon pair sur le réseau.
Parcourt un à un les tlv présents dans la liste du datagramme reçu et construit un tlv-réponse pour chacun (sauf comportement spécial selon type de tlv)
Accumule les tlv-réponses dans une liste chaînée de tlv, quand cette liste chaînée est pleine elle est envoyée.
*/
void respond_to_dtg (struct dtg_t *dtg, int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {

	struct tlv_t *tlv = dtg->tlv_list; // <-- pointeur qui parcourt la liste de tlvs REÇUS
	struct tlv_t *response_tlv_list = NULL; // <-- liste chainée des tlvs À ENVOYER
	struct tlv_t *response_tlv; // <-- variable reçevant les tlv de réponses des différentes fonctions
	struct tlv_t **pointer = &response_tlv_list; // <-- pointeur qui parcourt la liste chainée des tlv à envoyer pour la créer
	int size_tlv_list = 0; // <--- mesure la taille en octets de la liste de tlv À ENVOYER
	int tlv_count = 0; // <--- compte le nombre de tlv présents dans la liste de tlv À ENVOYER

	while(tlv != NULL) {
		response_tlv = respond_to_tlv(tlv, sockfd, from, size_from, peer_state);

		if(response_tlv == NULL) {
			tlv = tlv->next;
			continue;
		}

		if (size_tlv_list + response_tlv->length + TLV_HEADER > 1020) { // Si il n'y a pas assez de place pour ce nouveau tlv, procéder à l'envoi
			struct tlv_t *test = response_tlv_list; // <----- DEBUG
			printf("SENDING LIST OF TLV : \n");
			while(test != NULL) {
				printf("Tlv type = %d\n", test->type);
				test = test->next;
			}

			send_tlv_list(sockfd, from, size_from, tlv_count, response_tlv_list);
			response_tlv_list = NULL; // <--- ATTENTION À FREE LA TLV LIST !! (fait dans send_tlv_list)
			pointer = &response_tlv_list;
			*pointer = response_tlv;
			size_tlv_list = (*pointer)->length + TLV_HEADER;
			tlv_count = 1;

		} else {
			// ajouter response_tlv à la liste.
			*pointer = response_tlv;
			size_tlv_list += (*pointer)->length + TLV_HEADER;
			tlv_count ++;
		}

		pointer = &((*pointer)->next); // <-- déplacer le curseur dans la liste chaînée
		tlv = tlv->next;
	}

	if (response_tlv_list != NULL) {
		struct tlv_t *test = response_tlv_list; // <----- DEBUG
		printf("SENDING LIST OF TLV : \n");
		while(test != NULL) {
			printf("Tlv type = %d\n", test->type);
			test = test->next;
		}

		send_tlv_list(sockfd, from, size_from, tlv_count, response_tlv_list);

	}
}
