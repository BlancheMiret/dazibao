#include "hash.h"

void print_hash(char *hash) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}


int hash(char *value_to_hash, size_t size_value_to_hash, char *final_hash) {
	unsigned char *res = SHA256((const unsigned char*)value_to_hash, size_value_to_hash, 0);
	memcpy(final_hash, res, 16);
	return 0;
}


/* seqno en format RÉSEAU DANS LES PARAMÈTRES*/
int hash_node(uint64_t node_id, uint16_t seq_no, char data[192], char *final_hash) {
	int TRIPLETSIZE = sizeof(uint64_t) + sizeof(uint16_t) + strlen(data);
	char triplet[TRIPLETSIZE];
	memcpy(triplet, &node_id, 8);
	memcpy(triplet+8, &seq_no, 2); //selon le sujet page 2, doit bien être en big-endian dans le hash
    memcpy(triplet+10, data, strlen(data));
    hash(triplet, TRIPLETSIZE, final_hash); // <--- hachage
    return 0;
}
