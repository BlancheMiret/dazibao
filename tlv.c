#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/select.h>
#include <net/if.h>
#define MSG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024




int main_datagram(char * msg){
    memset(msg, 0, SIZE);
	uint8_t magic = 0x5F;
	uint8_t ver = 0x1;
	memcpy(msg, &magic, 1);
	memcpy(msg+1, &ver, 1);
	return MSG_HEADER;


}


/****** longueur du message principal *******/

uint16_t length_body(char *messageTLV)
{
    uint16_t length;
    memcpy(&length, messageTLV + 2, 2);
    uint16_t len = ntohs(length);
    return len;
}

int msgBodyTosend(char *message, char *body, uint16_t len)
{

    uint16_t convert_len = ntohs(length_body(message));

    memcpy(message + 4 + convert_len, body, len);

    convert_len = convert_len + len;

    uint16_t convert_h = htons(convert_len);
    memcpy(message + 2, &convert_h, 2);

    return MSG_HEADER + convert_len;
}



//TLV

uint8_t getTLV_TYPE(char * tlv) {
	uint8_t type;
	memcpy(&type, tlv, 1);
	return type;
}

uint8_t getTLV_LENGTH(char * tlv) {
	uint8_t len;
	memcpy(&len, tlv+1, 1);
	return len;
}

char * getTLV_BODY(char * tlv) {
	return tlv+TLV_HEADER;
}

//Création Pad1


int Pad1(char * pad) {
	memset(pad, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x0;
	memcpy(pad, &type, 1);
	return 1;
}

//Création de PadN
int PadN(char * pad, uint8_t len) {
	memset(pad, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x1;
	memcpy(pad, &type, 1);
	memcpy(pad+1, &len, 1);
	return TLV_HEADER+len;
}


//Creation de Neighbour request


int Neighbour_request(char * neighbourReq) {
	memset(neighbourReq, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x2;
	uint8_t len = 0x0;
	memcpy(neighbourReq, &type, 1);
	memcpy(neighbourReq+1, &len, 1);
	return TLV_HEADER;
}



//Creation de Neighbour 


int Neighbour(char * neigbour, struct in6_addr IP, in_port_t port) {
	memset(neigbour, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x3;
	//len = 18
	uint8_t len = 0x12;
	memcpy(neigbour, &type, 1);
	memcpy(neigbour+1, &len, 1);
	memcpy(neigbour+2, &IP, 16);
	memcpy(neigbour+18, &port, 2);
	return len+TLV_HEADER;
}



//Creation de Network Hash  


int Network_hash(char * network_hash, char * hash){
    memset(network_hash, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x4;

 	uint8_t len = 0x0;
	memcpy(network_hash, &type, 1);
	memcpy(network_hash+1, &len, 1);
	return len+TLV_HEADER;


}

//Creation de Network State Request


int Network_state_request(char * network_req){
    memset(network_req, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x5;

 	uint8_t len = 0x0;
	memcpy(network_req, &type, 1);
	memcpy(network_req+1, &len, 1);
	return len+TLV_HEADER;


}

//Creation de Node Hash

int Node_hash(char * node_hash, uint64_t node_id, uint16_t seqno, char * hash){
    memset(node_hash, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x6;
    //len = 26
 	uint8_t len = 0x1A;
	memcpy(node_hash, &type, 1);
	memcpy(node_hash+1, &len, 1);
	memcpy(node_hash+2, &node_id, 8);
	memcpy(node_hash+10, &seqno, 2);
	memcpy(node_hash+12, &hash, 16);
	return len+TLV_HEADER;


}

//Creation de Node State Request


int Node_state_request(char * node_state_req, uint64_t node_id ){

    memset(node_state_req, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x7;
    //len = 26
 	uint8_t len = 0x8;
	memcpy(node_state_req, &type, 1);
	memcpy(node_state_req+1, &len, 1);
	memcpy(node_state_req+2, &node_id, 8);

	return len+TLV_HEADER;

}


//Création de Node State

int Node_state(char * nodestate, uint64_t node_id, uint16_t seqno, char * node_hash, char * data, int data_length) {
	memset(nodestate, 0, SIZE-MSG_HEADER);

	uint8_t type = 0x8;
	//len = 28 sans data length
 	uint8_t len = 0x1C + data_length;
	memcpy(nodestate, &type, 1);
	memcpy(nodestate+1, &len, 1);
	memcpy(nodestate+2, &node_id, 8);
	memcpy(nodestate+10, &seqno, 2);
	memcpy(nodestate+12, &node_hash, 16);
	memcpy(data+28, data, data_length);
	return len+TLV_HEADER;
}


//Creation de warning

int Warning(char * warning, char * message, int message_length) {
	memset(warning, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x9;
	uint8_t len = message_length;
	memcpy(warning, &type, 1);
	memcpy(warning+1, &len, 1);
	memcpy(warning+2, message, message_length);
	return len+TLV_HEADER;
}



/****************************************************************************/

