#include "neighbour.h"

// ----------------------------------------------------------------------------
// --------------------------------- ECRITURE ---------------------------------

/* Ajoute un voisin dans neighbour_table associé à la socket key et dont "last_reception" est initilisé au temps courant
Renvoie -1 en cas d'erreur, 0 sinon */
int add_neighbour(struct neighbour *neighbour_table, struct sockaddr_storage *key, int perm) { 
	int i = 0;
	while(neighbour_table[i].exists && i < NBMAX) i++;

	if (i == NBMAX) {
		perror("N:20  - Already 15 neighbour.\n");
		return -1;
	}

	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		return -1;
	}	

	neighbour_table[i].exists = 1;
	neighbour_table[i].permanent = perm;
	neighbour_table[i].last_reception = tp;
	neighbour_table[i].socket_addr = *key;

	char IP2[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&neighbour_table[i].socket_addr)->sin6_addr), IP2, INET6_ADDRSTRLEN);
	printf("N:39  - Voisin ajouté. IP : %s\n", IP2);

	return 0;
}


/* Met à jour à l'heure courante le champ last_reception du neighbour de socket_addr key
Si un tel voisin n'existe pas, renvoie une erreur
Renvoie -1 en cas d'erreur, 0 sinon */
int update_last_reception(struct neighbour *neighbour_table, struct sockaddr_storage *key) { 
	
	int i = find_neighbour(neighbour_table, key);

	if (i == -1) {
		perror("Key not present in hashtable.\n");
		return -1;
	}

	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		return -1;
	}

	neighbour_table[i].last_reception = tp;
	return 0;
}


/* Supprime chaque voisin de neighbour_table dont la dernière réception remonte à plus de 70 secondes
 Retourne le nombre d'éléments supprimés */
int sweep_neighbour_table(struct neighbour *neighbour_table) {
	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		exit(1);
	}

	int nb_deleted = 0;
	for (int i = 0; i < NBMAX; i++) {
		if (neighbour_table[i].exists && !neighbour_table[i].permanent && (tp.tv_sec - neighbour_table[i].last_reception.tv_sec > 70)) {
			neighbour_table[i].exists = 0;
			nb_deleted +=1;
		}
	}
	return nb_deleted;
}


// ----------------------------------------------------------------------------
// ---------------------------------- LECTURE ---------------------------------

/* Renvoie 1 si les x et y représentent la même adresse de socket, 0 sinon */
int struct_addr_equals(struct sockaddr_storage *x, struct sockaddr_storage *y) {
	if (x->ss_family != y->ss_family) return 0; // FALSE

	if (x->ss_family == AF_INET) {
		struct sockaddr_in *x4 = (void*)x;
		struct sockaddr_in *y4 = (void*)y;
		if(ntohl(x4->sin_addr.s_addr) != ntohl(y4->sin_addr.s_addr)) return 0;
		if(ntohs(x4->sin_port) != ntohs(y4->sin_port)) return 0;
		return 1;
	}

	if (x->ss_family == AF_INET6) {
		struct sockaddr_in6 *x6 = (void*)x;
		struct sockaddr_in6 *y6 = (void*)y;
		int r = memcmp(x6->sin6_addr.s6_addr, y6->sin6_addr.s6_addr, sizeof(struct in6_addr));
		if (r != 0) {
			char IP1[INET6_ADDRSTRLEN];
			char IP2[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(x6->sin6_addr), IP1, INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(y6->sin6_addr), IP2, INET6_ADDRSTRLEN);
			return 0;
		}
		if(ntohs(x6->sin6_port) != ntohs(y6->sin6_port)) return 0;
		return 1;
	}

	perror("Unknown socket family\n");
	return -1;
}


/* Renvoie l'indice dans neighbour_table du voisin de socket_addr key, -1 si le voisin n'est pas trouvé */
int find_neighbour(struct neighbour *neighbour_table, struct sockaddr_storage *key) {
	for (int i = 0; i < NBMAX; i++) {
		if (struct_addr_equals(&neighbour_table[i].socket_addr, key)) return i;
	}
	return -1;
}


/* Renvoie l'adresse d'un voisin choisi au hasard dans neighbour_table */
struct neighbour * pick_neighbour(struct neighbour *neighbour_table){ 
	srand(time(NULL));
	while(1) {
		int r = rand()%15;
		if(neighbour_table[r].exists) return &neighbour_table[r];

	}
}


/* Renvoie le nombre de voisins dans neighbour_table */
int get_nb_neighbour(struct neighbour *neighbour_table) { 
	int count = 0;
	for (int i = 0; i < NBMAX; i++) {
		if (neighbour_table[i].exists) count += 1;
	}
	return count;
}


// ----------------------------------------------------------------------------
// -------------------------------- AFFICHAGE ---------------------------------

/* Prend une struct timeval et l'affiche au format annee/mois/jour heure:minutes:secondes */
void print_real_time(struct timeval tv){

	struct tm* ptm;
	char time_string[40];

	ptm = localtime (&tv.tv_sec);
	strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
	printf ("%s.\n", time_string);
}


/* Affiche le voisin d'adresse n*/
void display_neighbour(struct neighbour *n) {

	struct sockaddr_storage *k = &(n->socket_addr);
	char IP[INET6_ADDRSTRLEN] = {0};
	char *family;
	int port;

	switch(n->socket_addr.ss_family) {
		case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)k)->sin_addr), IP, INET_ADDRSTRLEN);
		family = "iPv4";
		port = ((struct sockaddr_in *)k)->sin_port;
		break;
		case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)k)->sin6_addr), IP, INET6_ADDRSTRLEN);
		family = "iPv6";
		port = ((struct sockaddr_in6 *)k)->sin6_port;
		break;
	}

	printf("--------- NEIGHBOUR --------- \n");
	printf("Is this neighbour permanent ? : %d\n", n->permanent);
	printf("Time of last reception : \n");
	print_real_time(n->last_reception);
	printf("- Socket informations\n");
	printf("sa_family is : %s\n", family);
	printf("port is %d\n", ntohs(port));
	printf("IP address is : %s\n", IP); 
	printf("\n");	

}

/* Affiche le contenu de la table neighbour_table 
neighbour_table doit être de taille NBMAX */
void display_neighbour_table(struct neighbour *neighbour_table) {
	int count = 0;
	for (int i = 0; i < NBMAX; i++) {
		if (neighbour_table[i].exists) {
			display_neighbour(&neighbour_table[i]);
			count++;
		}
	}
	if (count == 0) printf("The table is empty.\n");
}


