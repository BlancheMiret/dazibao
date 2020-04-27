#ifndef __HASH_H__
#define __HASH_H__

/*
BESOINS
- Calculer un hash

FONCTIONS
 Peut - petre juste module de calcul de hash après tout

- int hash(char *to_hash, char hash[16]); // <-- met à jour valeur à l'adresse hash
	 // <-- utilise SHA256 et tronque à 16 octets, les autres foncitons concatènent les données à hacher. Fonction interne

- int hash_node(uint64_t node_id, uint16_t seq_no, char data[192], char node_hash[16]); // <-- met à jour valeur à l'adresse node_hash
- int hash_network(nstate_t node_state, char network_hash[16]); // <-- met à jour valeur à l'adresse network_hash

*/

#endif