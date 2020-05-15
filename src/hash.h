#ifndef __HASH_H__
#define __HASH_H__
#include <inttypes.h>

/* Cacule SHA256 de value_to_hash et en écrit les 16 premiers octets à l'adresse final_hash */
int hash(char *value_to_hash, size_t size_value_to_hash, char *final_hash);

/* Calcule le hash du noeud d'id node_id, de numéro de séquence seq_no et de donnée data, et l'écrit à l'adresse final_hash. */
int hash_node(uint64_t node_id, uint16_t seq_no, char data[192], char *final_hash);

/* Renvoie 1 si hash1 = hash2, 0 sinon */
int compare_2_hash(char hash1[16], char hash2[16]);

void print_hash(char *hash);

#endif
