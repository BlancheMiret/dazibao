#include "inondation.h"

// ----------------------------------------------------------------------------
// ------------------------------ FREE TLV LIST -------------------------------

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

void *respond_to_neighbour_req(struct pstate_t *peer_state) {
	// tire au hasard entrée table des voisins
	struct neighbour *n = pick_neighbour(peer_state->neighbour_table); // <--- fonction à modifier, renvoie int !!
	
	// Créer TLV Neighbour contenant IP et Port du Neighbour choisi 
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&n->socket_addr;
	struct tlv_t *tlv_neighbour = new_neighbour(sin6->sin6_addr,sin6->sin6_port);

	return tlv_neighbour;
}


// ----------------------------------------------------------------------------
// --------------------------- RESPOND TO NEIGHBOUR ---------------------------

void *respond_to_neighbour(struct pstate_t *peer_state) {
	return new_network_hash(peer_state->network_hash);
}


// ----------------------------------------------------------------------------
// ------------------------- RESPOND TO NETWORK HASH --------------------------

// UN SEUL TLV DE RÉPONSE

void *respond_to_network_hash(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 4) {
		printf("Shouldn't be in respond_to_network_hash function.\n");
		return NULL;
	}

	// Si hash du réseau identique , rien à faire, sinon envoyer TLV NETWORK STATE REQUEST
	char received_hash[16];
	memcpy(received_hash, tlv->body.nethash_body->network_hash, 16);
	char known_hash[16];
	memcpy(known_hash, peer_state->network_hash, 16);

	if (compare_2_hash(received_hash, known_hash)) return NULL; // fonction dans module hash

	return new_network_state_request();
}


// ----------------------------------------------------------------------------
// --------------------- RESPOND TO NETWORK STATE REQUEST ---------------------

// MILLE TLV EN RÉPONSE, SE DÉBROUILLE


// ON SAIT QU'UN NODE HASH MESURE 28 OCTETS 
// ATTENTION À NE PAS EN METTRE TROP DANS UN DATAGRAMME !!!
// AU MAX : 1024 - 4 (DTG HEADER) = 1000 / 28 = 35 
// On ne va pas ajouter manuellement les 35 tlv dans la fonction build_char_to_tlvs...
// --> solution : nouvelle fonction build_tlvs_to_char2 qui prend une liste chainée de tlv plutôt que chaque tlv en argument

int respond_to_network_state_req(int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {
	// Réponse = Envoyer une série de Node Hash, un pour chaque donnée connue.

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

// consulte table de données
// s'il n'a pas d'entrée pour l'id, ou que les hash sont différérents : envoyer node state request
// sinon, hash égaux : ne rien faire
void *respond_to_node_hash(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 6) {
		printf("Shouldn't be in respond_to_ node_hash function.\n");
		return NULL;
	}

	int rc = compare_hash(peer_state->data_table, tlv->body.nodehash_body->node_id, tlv->body.nodehash_body->node_hash); // <-- fonction à vérifier. Notamment : si pas d'entrée, renvoie bien 0 (faux) ?
	if (rc) return NULL;
	return new_node_state_request(tlv->body.nodehash_body->node_id);

}


// ----------------------------------------------------------------------------
// ----------------------- RESPOND TO NODE STATE REQUEST ----------------------

// UN SEUL TLV EN RÉPONSE

void *respond_to_node_state_request(struct tlv_t *tlv, struct pstate_t *peer_state) {
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

// PAS DE RÉPONSE DE TLV, SEULEMENT ADD DATA

int respond_to_node_state(struct tlv_t *tlv, struct pstate_t *peer_state) {
	if (tlv->type != 7) {
		printf("Shouldn't be in respond_to_node_state function.\n");
		return -1;
	}
	int rc = compare_hash(peer_state->data_table, tlv->body.nodestate_body->node_id, tlv->body.nodestate_body->node_hash); // <-- fonction à vérifier. Notamment : si pas d'entrée, renvoie bien -1 ?
	if (rc) return 0;

	uint64_t neigh_node_id = tlv->body.nodestate_body->node_id;
	uint16_t neigh_seq_no = tlv->body.nodestate_body->seq_no; 

	// si iota = self id
	if (neigh_node_id == peer_state->node_id) {
		update_self_seq_num(peer_state->data_table, peer_state->node_id);
		return 0;
	}

	// iota != self id
	struct data_t *value = g_hash_table_lookup(peer_state->data_table, &neigh_node_id);
	if(value == NULL || !(is_greater_than(ntohs(value->seq_no), ntohs(neigh_seq_no)))) { // 
		// stocker donnée (avec données node state) dans table des données
		add_data(peer_state->data_table, neigh_node_id, neigh_seq_no, tlv->body.nodestate_body->data);
	}

	// Si numéro de séquence supérieur chez nous : ne rien faire

	return 0;
}


// ----------------------------------------------------------------------------
// ------------------------------- RESPOND TO TLV -----------------------------


void *respond_to_tlv(struct tlv_t *tlv, int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {
	struct tlv *response_tlv = NULL;
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
			response_tlv = respond_to_neighbour_req(peer_state); // <- récupère un tlv_neighbour
			break;

		case 3: // Neigbhour
			response_tlv = respond_to_neighbour(peer_state); // <- récupère network hash
			break;

		case 4: // Network Hash
			response_tlv = respond_to_network_hash(tlv, peer_state);  // <- récupère tlv network state request ou NULL
			break;

		case 5: // Network State Request
			respond_to_network_state_req(sockfd, from, size_from, peer_state); // <- le seul qui fait ses envois tout seul, sinon compliqué...
			break;

		case 6: // Node Hash
			response_tlv = respond_to_node_hash(tlv, peer_state); // <- récupère Node State Request ou NULL
			break;

		case 7: // Node State Request
			response_tlv = respond_to_node_state_request(tlv, peer_state); //<- récupère Node State ou Null (si erreur)
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

// IDÉE POUR ÉCONOMISER LES SEND TO :
// STOCKER DANS UNE LISTE CHAINÉE (EN VÉRIFIANT QUE TAILLE DÉPASSE PAS 1020 OCTESTS)
// QUAND LISTE EST PLEINE : SEND TO
// LIBÉRER LA LISTE CHAINÉE
// RECOMMENCER S'IL RESTE DES TLVs


void respond_to_dtg (struct dtg_t *dtg, int sockfd, struct sockaddr_in6 *from, size_t size_from, struct pstate_t *peer_state) {
	struct tlv_t *tlv = dtg->tlv_list; // <-- pointeur qui parcourt la liste de tlvs REÇUS
	struct tlv_t *response_tlv_list = NULL; // <-- liste chainée des tlvs À ENVOYER
	struct tlv_t *response_tlv; // <-- variable reçevant les tlv de réponses des différentes fonctions
	struct tlv_t **pointer = &response_tlv_list; // <-- pointeur qui parcourt la liste chainée des tlv à envoyer pour la créer
	int size_tlv_list = 0; // <--- mesure la taille en octets de la liste de tlv À ENVOYER
	int tlv_count = 0; // <--- compte le nombre de tlv présents dans la liste de tlv À ENVOYER

	while(tlv != NULL) {
		response_tlv = respond_to_tlv(tlv, sockfd, from, size_from, peer_state);

		if(response_tlv == NULL) continue;

		if (size_tlv_list + response_tlv->length + TLV_HEADER > 1020) { // Si il n'y a pas assez de place pour ce nouveau tlv, procéder à l'envoi

			send_tlv_list(sockfd, from, size_from, tlv_count, response_tlv_list);
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
}

