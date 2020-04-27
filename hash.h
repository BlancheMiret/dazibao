#ifndef __HASH_H__
#define __HASH_H__

/*
BESOINS
- Calculer un hash

FONCTIONS
 Peut - petre juste module de calcul de hash après tout

- char[16] hash(char *to_hash); // <-- utilise SHA256 et tronque à 16 octets, les autres foncitons concatènent les données à hacher. Fonction interne

- char[16] hash_node(uint64_t node_id, uint16_t seq_no, char data[192]);
- char[16] hash_network(nstate_t node_state);

*/

#endif