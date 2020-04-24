#include <openssl/sha.h>
#include "tlv_manager.h"

int main() {
	// ---- adresse IP et port
	in_port_t port = htons(1717);
	char *ip_str = "2001:db8:0:85a3:0:0:ac1f:8001";
	struct in6_addr ip;
	inet_pton(AF_INET6, ip_str, &ip);

	// ---- node_id, num seq, data et hash
	char *data = "J'ai passé une excellente soirée mais ce n'était pas celle-ci.";
	uint16_t seq_no = htons(0x3E08); // 0x3D = 61 --- 0x3E08 = 15880 
	uint64_t node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);

	int TRIPLETSIZE = sizeof(uint64_t) + sizeof(uint16_t) + strlen(data);
    char triplet[TRIPLETSIZE]; 
    memcpy(triplet, &node_id, 8);
    memcpy(triplet+8, &seq_no, 2);
    memcpy(triplet+10, data, strlen(data));
	unsigned char *res = SHA256((const unsigned char*)triplet, TRIPLETSIZE, 0);
    char node_hash[16];
    memcpy(node_hash, res, 16);

    //----

    struct tlv_t *tlv = new_pad1();
	//print_tlv(tlv);

	struct tlv_t *tlv9 = new_padN(18);
	//print_tlv(tlv9);

	struct tlv_t *tlv8 = new_neighbour_request();
	//print_tlv(tlv8);
	
	struct tlv_t *tlv7 = new_neighbour(ip, port);
	//print_tlv(tlv7);

	struct tlv_t *tlv6 = new_network_hash(node_hash);
	//print_tlv(tlv6);

	struct tlv_t *tlv5 = new_network_state_request();
	//print_tlv(tlv5);

	struct tlv_t *tlv4 = new_node_hash(node_id, seq_no, node_hash);
	//print_tlv(tlv4);

	struct tlv_t *tlv3  = new_node_state_request(node_id);
	//print_tlv(tlv3);

	struct tlv_t *tlv1 = new_warning("I AM NOT HAPPY THIS IS A SERIOUS WARNING");
	//print_tlv(tlv1);

	struct tlv_t *tlv2 = new_node_state(node_id, seq_no, node_hash, data);
	print_tlv(tlv2);

	printf("----- DEBUG BUILD TLVS TO CHAR -----\n");
	int size_dtg;
	char *dtg_char = build_tlvs_to_char(&size_dtg, 10, tlv, tlv1, tlv2, tlv3, tlv4, tlv5, tlv6, tlv7, tlv8, tlv9);


	printf("--------- DEBUG UNPACK TLV ---------\n");

	struct dtg_t *dtg = unpack_dtg(dtg_char, size_dtg);
	print_dtg(dtg);


}









