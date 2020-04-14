//#include <sys/types.h>
//#include <sys/uio.h>
//#include <unistd.h>

#include <stdlib.h> //exit
#include <stdio.h> //perror, snprintf
//#include <string.h>
//#include <fcntl.h> //open

#include <glib.h>
#include <glib/gprintf.h>

//#include <sys/socket.h>
//#include <netinet/in.h>

//#include <time.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <netdb.h>

#include <arpa/inet.h>

struct neighbour {
		int 				permanent;
		struct timeval		last_reception;
	};

void print_neighbour(struct neighbour n) {
	//printf("-------- Displaying struct neighbour --------\n");
	printf("Is this neighbour permanent ? : %d\n", n.permanent);
	printf("Time of last reception : \n");
	printf("Seconds since Jan. 1, 1970 : %ld\n", n.last_reception.tv_sec);
	printf("Microseconds since Jan. 1, 1970 : %d\n", n.last_reception.tv_usec);
}

void print_sockaddr(struct sockaddr_in6 s) {
	//printf("-------- Displaying struct sockaddr_in6 --------\n");
	printf("sa_family is : %d\n", s.sin6_family);
	printf("port is %d\n", s.sin6_port);
}

int main() {

	GHashTable *ht = g_hash_table_new(NULL, NULL);

	/* // TEST SIMPLE - pas d'allocation

	int k = 8;
	int v = 19;

	g_hash_table_insert(ht, &k, &v);

	int *r = g_hash_table_lookup(ht, &k);

	printf("%d\n", *r);

	*/

	// TEST AVEC STRUCTURES - pas d'allocation

	// création d'un voisin qui sera une valeur dans la hashtable
	struct neighbour v1;
	memset(&v1, 0, sizeof(struct neighbour));
	v1.permanent = 0;
	struct timeval tp;
	if (gettimeofday(&tp, NULL) < 0) {
		perror("gettimeofday");
		exit(1);
	}
	v1.last_reception = tp;

	// création d'une struct sockaddr qui sera une clé dans la hashtable
	struct sockaddr_in6 k1;
	memset(&k1, 0, sizeof(struct sockaddr_in6));

	printf("------------ SHOWING ORIGINAL VALUE ------------ \n");
	print_neighbour(v1);

	g_hash_table_insert(ht, &k1, &v1);
	struct neighbour *r = g_hash_table_lookup(ht, &k1);

	printf("------------- SHOWING HASHED VALUE ------------- \n");
	print_neighbour(*r);

}




