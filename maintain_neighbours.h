#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <net/if.h>
#include <locale.h>
#include <openssl/sha.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "peer_state.h"
#include "tlv_manager.h"

void send_neighbour_req(int socket, struct pstate_t * peer_state);
int compare_addr(struct in6_addr *IP1, struct in6_addr *IP2);
void maintain_neighbour_table(struct pstate_t * peer_state, struct sockaddr_in6 from);
void send_network_hash(int socket, struct pstate_t * peer_state);

