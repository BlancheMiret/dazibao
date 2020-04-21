#include <stdio.h>
#include <arpa/inet.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <string.h>
#include "neighbour.h"

int main() {
	GHashTable *ntbl = create_neigh_table();

	printf("\n---------------------- EMPTY TABLE ----------------------\n\n");

	display_neighbour_table(ntbl);

	printf("\n------------------- ADD 1ST & 2ND VALUE -------------------\n\n");

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(struct sockaddr_in6));
	sin6.sin6_family = AF_INET6; 
	sin6.sin6_port = 1616;

	struct sockaddr_in6 sin2;
	memset(&sin2, 0, sizeof(struct sockaddr_in6));
	sin2.sin6_family = AF_INET6; 
	sin2.sin6_port = 1717;

	add_neighbour(ntbl, (struct sockaddr*)&sin6, 1);
	add_neighbour(ntbl, (struct sockaddr*)&sin2, 0);
	display_neighbour_table(ntbl);


	sleep(25);

	printf("\n---------------------- ADD 3D VALUE ----------------------\n\n");

	struct sockaddr_in sin3;
	memset(&sin3, 0, sizeof(struct sockaddr_in));
	sin3.sin_family = AF_INET; 
	sin3.sin_port = 1818;

	add_neighbour(ntbl, (struct sockaddr*)&sin3, 0);
	display_neighbour_table(ntbl);

	sleep(25);

	printf("\n-------------------- UPDATE 3D VALUE ---------------------\n\n");

	update_last_reception(ntbl, (struct sockaddr*)&sin3);
	display_neighbour_table(ntbl);

	neighbour_table_iter(ntbl);

	sleep(25);

	printf("\n----------------------- SWEEP TABLE ----------------------\n\n");

	printf("%d elements deleted.\n", sweep_neighbour_table(ntbl));
	display_neighbour_table(ntbl);

	printf("Table len : %d\n",get_table_len(ntbl));
}