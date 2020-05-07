#include "data_manager.h"
#include "hash.h"

int main() {

	char *data = "J'ai passé une excellente soirée mais ce n'était pas celle-ci.";
	uint16_t net_seq_no = htons(0x3E08); //15880
	uint64_t node_id =
	(((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	(((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	(((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	(((uint64_t) rand() << 48) & 0xFFFF000000000000ull);

	printf("Hello World.\n");

	printf("---------- CREATE EMPTY DATA TABLE ----------\n");
	GHashTable *data_table = create_data_table();
	printf("Table len = %d\n", get_data_table_len(data_table));
	display_data_table(data_table);
	printf("\n");

	printf("--------------- ADD ONE VALUE ---------------\n");
	add_data(data_table, node_id, net_seq_no, data);
	printf("Table len = %d\n", get_data_table_len(data_table));
	display_data_table(data_table);
	printf("\n");

	printf("---- GET THE VALUE WITH EQUIVALENT KEY ? ----\n");
	uint64_t node_id2;
	memcpy(&node_id2, &node_id, 8);
	print_data(data_table, node_id2);
	printf("\n");

	printf("----- GET SEQ NUM WITH EQUIVALENT KEY ? -----\n");
	uint64_t *node_id3 = malloc(8);
	memcpy (node_id3, &node_id, 8);
	uint16_t host_seq_no = get_seq_no(data_table, *node_id3);
	printf("Seq no : %"PRIu16"\n", host_seq_no);
	printf("\n");

	printf("--------------- COMPARE HASH ? --------------\n");
	char hash_node_to_compare[16];
	hash_node(node_id, net_seq_no, data, hash_node_to_compare);
	if (compare_hash(data_table, node_id2, hash_node_to_compare)) printf("Hash are equal as they should be.\n");
	else printf("Hash are not equal. Something's wrong.\n");
	printf("\n");

	printf("------------- UPDATE THE VALUE ? ------------\n");
	uint16_t new_net_seq_no = htons(0x7CB); //1995
	printf("IN NETWORK ORDER, 1995 = %"PRIu16"\n", new_net_seq_no);
	char *new_data = "Sans petit bisou les rêves sont flous.";
	add_data(data_table, node_id, new_net_seq_no, new_data);
	printf("Table len = %d\n", get_data_table_len(data_table));
	display_data_table(data_table);
	printf("\n");

	printf("--------------- COMPARE HASH ? --------------\n");
	hash_node(node_id, new_net_seq_no, new_data, hash_node_to_compare);
	if (compare_hash(data_table, node_id2, hash_node_to_compare)) printf("Hash are equal as they should be.\n");
	else printf("Hash are not equal. Something's wrong.\n");
	printf("\n");

	printf("-------------- ADD NEW VALUE ? --------------\n");
	uint64_t new_node_id = 19;
	add_data(data_table, new_node_id, net_seq_no, data);
	printf("Table len = %d\n", get_data_table_len(data_table));
	display_data_table(data_table);
	printf("\n");

	printf("----------- UPDATE SELF SEQ NUM ? -----------\n");
	uint16_t newseqno = htons(2005);
	update_self_seq_num(data_table, node_id, newseqno);
	printf("Table len = %d\n", get_data_table_len(data_table));
	display_data_table(data_table);
	printf("\n");

	printf("------------- IS GREATER THAN ? -------------\n");
	uint16_t host_seq_no1 = get_seq_no(data_table, node_id); //1 996 à ce stade
	uint16_t host_seq_no2 = get_seq_no(data_table, new_node_id); //15 880
	if(is_greater_than(host_seq_no2, host_seq_no1)) printf("Expected result.\n");
	else printf("Something's wrong.\n");
	printf("\n");


}






