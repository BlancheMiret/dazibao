
/*
Besoins :
- toutes les 20 secondes, faire le ménage de la table des voisins (supprimer ceux dont pas de nouvelle depuis longtemps)
- connaître le nombre de voisins dans la table des voisins (toutes les 20 secondes, si <5, envoyer un TLV Neighbour Request à voisin au hasard )
- tirer un neighbour au hasard (donc renvoyer une struct sockaddr)

*/

void* create_neigh_table();
int get_table_len(GHashTable *neighbour_table);
int add_neighbour(GHashTable *neighbour_table, struct sockaddr_in6 *key, int perm);
int update_last_reception(GHashTable *neighbour_table, struct sockaddr_in6 *key);
int sweep_neighbour_table(GHashTable *neighbour_table);
void display_neighbour_table(GHashTable *neighbour_table);