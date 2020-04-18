#define MSG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024



void* main_datagram();
uint16_t get_body_length(char * message);
int check_datagram_header(char * msg_received);
int set_msg_body(char *message, char *body, uint16_t len);
uint8_t get_tlv_type(char * tlv);
uint8_t get_tlv_length(char * tlv);
char * get_tlv_body(char * tlv);
int Pad1(char * pad);
int PadN(char * pad, uint8_t len);
int Neighbour_request(char * neighbourReq);
int Neighbour(char * neigbour, struct in6_addr IP, in_port_t port);
int Network_hash(char * network_hash, char * hash);
int Network_state_request(char * network_req);
int Node_hash(char * node_hash, uint64_t node_id, uint16_t seqno, char * hash);
int Node_state_request(char * node_state_req, uint64_t node_id );
int Node_state(char * nodestate, uint64_t node_id, uint16_t seqno, char * node_hash,  char * data, size_t data_length);
int Warning(char * warning, char * message, int message_length);
void print_datagram(char *msg);

