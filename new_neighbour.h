#ifndef __NEW_NEIGHBOUR_H__
#define __NEW_NEIGHBOUR_H__

int get_table_len(GHashTable *neighbour_table);
int add_neighbour(GHashTable *neighbour_table, struct sockaddr *key, int perm);
int update_last_reception(GHashTable *neighbour_table, struct sockaddr *key);
int sweep_neighbour_table(GHashTable *neighbour_table);
void display_neighbour_table(GHashTable *neighbour_table);
void neighbour_table_iter(GHashTable *neighbour_table);

#endif