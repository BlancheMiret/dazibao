#include <stdio.h>
#include <arpa/inet.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <string.h>
#include "neighbour.h"

int main() {

	struct neighbour ntbl[15];
	memset(ntbl, 0, 15 * sizeof(struct neighbour));

	printf("\n---------------------- EMPTY TABLE ----------------------\n\n");

	display_neighbour_table(ntbl);

	printf("\n------------------- ADD 1ST & 2ND VALUE -------------------\n\n");

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(struct sockaddr_in6));
	sin6.sin6_family = AF_INET6; 
	sin6.sin6_port = htons(1616);

	struct sockaddr_in6 sin2;
	memset(&sin2, 0, sizeof(struct sockaddr_in6));
	sin2.sin6_family = AF_INET6; 
	sin2.sin6_port = htons(1717);

	add_neighbour(ntbl, (struct sockaddr_storage*)&sin6, 1);
	add_neighbour(ntbl, (struct sockaddr_storage*)&sin2, 0);
	display_neighbour_table(ntbl);


	sleep(25);

	printf("\n---------------------- ADD 3D VALUE ----------------------\n\n");

	struct sockaddr_in sin3;
	memset(&sin3, 0, sizeof(struct sockaddr_in));
	sin3.sin_family = AF_INET; 
	sin3.sin_port = htons(1818);

	add_neighbour(ntbl, (struct sockaddr_storage*)&sin3, 0);
	display_neighbour_table(ntbl);

	sleep(25);

	printf("\n-------------------- UPDATE 3D VALUE ---------------------\n\n");

	update_last_reception(ntbl, (struct sockaddr_storage*)&sin3);
	display_neighbour_table(ntbl);

	sleep(25);

	printf("\n----------------------- SWEEP TABLE ----------------------\n\n");

	printf("%d elements deleted.\n", sweep_neighbour_table(ntbl));
	display_neighbour_table(ntbl);

	printf("Table len : %d\n",get_nb_neighbour(ntbl));
}