#include "hash.h"


/* Cacule SHA256 de value_to_hash et en écrit les 16 premiers octets à l'adresse final_hash */
int hash(char *value_to_hash, size_t size_value_to_hash, char *final_hash) {
	unsigned char *res = SHA256((const unsigned char*)value_to_hash, size_value_to_hash, 0);
	memcpy(final_hash, res, 16);
	return 0;
}


/* Calcule le hash du noeud d'id node_id, de numéro de séquence seq_no et de donnée data, et l'écrit à l'adresse final_hash.
Seqno doit être passé en format réseau */
int hash_node(uint64_t node_id, uint16_t seq_no, char data[192], char *final_hash) {
	int TRIPLETSIZE = sizeof(uint64_t) + sizeof(uint16_t) + strlen(data);
	char triplet[TRIPLETSIZE];
	memcpy(triplet, &node_id, 8);
	memcpy(triplet+8, &seq_no, 2); //selon le sujet page 2, doit bien être en big-endian dans le hash
    memcpy(triplet+10, data, strlen(data));
    hash(triplet, TRIPLETSIZE, final_hash); // <--- hachage
    return 0;
}


/* Renvoie 1 si hash1 = hash2, 0 sinon */
int compare_2_hash(char hash1[16], char hash2[16]) {
	for(int i = 0; i < 16; i++) {
		if (hash1[i] != hash2[i]) return 0;
	}
	return 1;
}


/* Affiche un hash */
void print_hash(char *hash) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", (unsigned char) hash[i]);
    }
    printf("\n");
}


