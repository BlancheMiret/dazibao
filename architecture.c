#include "tlv.h"

#define DTGSIZE 1024

/* ----------------------------- MODULE TLV ----------------------------- */

// IDÉE : CRÉER DES STRUCTURES POUR CHAQUE TLV  <---------------------------------------------------------------------------------- ?????
// et une fonction "build" transformant une structure en char* tab manipulable dans le réseau 
// --> sera plus facile de manipuler les tlv par structure dans le programme ?

void unpack_dtg(char *datagramme); // découpe le datagramme en tlv (liste chainée), s'occupe de la réaction, envoi les réponses nécessaires.


/* ----------------------------------------------------------------------------- */
/* ----------------------------- MODULE VOISINS ----------------------------- */


// structure qui représente un voisin dans la table des voisins (cf sujet p7)
struct neighbour {
	int 				permanent;
	struct timeval		last_reception;
};

// hashmap de la table des voisins 
typedef ... neighbour_table;
// Indexée par ?? Clé = structsockaddr ; ou clé = concaténation adresseIP + port ? Types ? <-------------------------- COMMENT INDEXER LES TABLE DES VOISINS ? 
// Valeur = de type struct neighbour;

void* getneighbour(? id_n);
void suppress_neighbour(? id_n);
int is_temporary(? id_n); //(boolean)
int is_too_old(? id_n); //(répond 0 (False) si secondes depuis last_reception < 70, 1 sinon)
? pick_neighbour(); //Renvoie un id de neighbour tiré au hasard


// ATTENTION, L'ID D'UN VOISIN DANS LA TABLE DES VOISINS N'EST PAS LE MÊME QUE L'ID DANS LA TABLE DES DATA.
// DATA indexe par le numéro d'id concret d'un voisin, NEIGHBOUR indexe par une adresse de socket, utilisable dans la gestion du réseau. 

/* ----------------------------------------------------------------------------- */
/* ----------------------------- MODULE DONNÉES -------------------------------- */


// structure qui représente une donnée dans la hashmap data_table (cf p7 sujet)
typedef data {
	uint16_t 	num_seq;
	char*		data;
} data;

typedef ... data_table; //hashmap avec clé = uint64_t node_id ; valeur = de type struct data


/* ----------------------------------------------------------------------------- */
/* ------------------------------ MODULE HASH ----------------------------------- */


void* h(int x); // la fonction qui récupère les 16 premiers octets (sur 32) de SHA256(x);
void* compute_node_hash(uint64_t node_id, ?? data_table); //-> peut chercher dans la table data_table pour créer le
void* compute_network_hash(?? data_table); //appelle compute_node_hash de chaque élément dans data_table... et renvoie nouveau network_hash


/* ----------------------------------------------------------------------------- */
/* ---------------------------------- MAIN -------------------------------------- */



int main() {

	/* ------------------ DONNÉES À MAINTENIR ---------------*/

	char* data; 
	uint16_t num_seq;
	uint64_t node_id;

	char* node_hash;

	/* ---------------- DONNÉES AMENÉES À ÉVOLUER -------------*/ 

	?? neighbour_table; 
	?? data_table;  

	// FAUT IL CRÉER UNE STRUCTURE CONTENANT CES MACHINS, STRUCTURE QUI SERAIT // <--------------------------------------------------------------- ????
	// PASSÉ EN PARAMÈTRE DE CHAQUE FONCTION POUR FAIRE CIRCULER ? 

	/* --------------- CREATION DE SOCKET ---------------------*/


	// À priori, une seule socket pour le noeud. 
	// Donc question : faisons nous tout en IPV6, avec des IPV4 mapped (voir setsockopt) ?? // --------------------------------------------------   ???
	// Attention : besoin de lier notre socket à un numéro de port, choisi arbitrairement ? 


	/* ---------------------- PROTOCOLE ------------------------*/

	// IDÉE : CRÉER DEUX PROCESSUS, UN QUI GÈRE LES ENVOIS (AVEC UNE BOUCLE SUR 20 SECONDES) // <-------------------------------------------------- ???? 
	// UN QUI GÈRE LES RÉCEPTIONS : AVEC UN SELECT SUR LES DESCRIPTEURS DE FICHIER... 
	// OU AUTREMENT (NOUVEAU PROCESSUS DÈS QUE RECEPTION DATAGRAMME )
	// (JE NE VOIS PAS TROP COMMENT FAIRE AVEC SELECT)


	/* GÉRER LES ENVOIS DE TLV INDÉPENDANTS DES RECEPTIONS */
	while(1) {

		// 1. Parcourir la table des voisins et éliminer voisins transitoires 

		// 2. Si moins de 5 voisins, envoyer un TLV Neighbour Request à un voisin tiré au hasard

		// 3. Envoyer un TLV Network Hash à tous les voisins 

		sleep(20); // toutes les 20 secondes 
	}


	/* GÉRER LES RÉCEPTIONS DE TLV ET L'ENVOI DE RÉPONSE */
	while(1) {

		/* C'est ici que ça se complique. */

		struct sockaddr sender;
		socklen_t sender_len;
		unsigned char datagramme[DTGSIZE];

		int rc = recvfrom(s, datagramme, DTGSIZE, 0, &sender, &sender_len); //<----------- fork dès que reception ?? Nouveau processus pour gérer chaque réception ?

		// 1. Vérifier si datagramme est correct : vérifier en-tête

		// 2. Identifier avec sender l'adresse de socket de l'envoyeur --> mettre à jour le champ last_reception dans la table des voisins ou ajouter voisin

		// 3. Analyser le datagramme et réagir (éventuellement renvoi)
		// Besoin de passer en paramètre la table des voisins, le node etc.
		unpack_dtg(datagramme, s);

		// 4. Fin de la réaction au datagramme reçu. (Si processus fils, fin du processus) ? 

	}



}