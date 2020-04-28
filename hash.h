#ifndef __HASH_H__
#define __HASH_H__

#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
//#include <glib.h>
//#include <glib/gprintf.h>

int hash_node(uint64_t node_id, uint16_t seq_no, char data[192], char *final_hash);
//int hash_network(GHashTable *data_table, char *final_hash);
void print_hash(char *hash);

#endif
