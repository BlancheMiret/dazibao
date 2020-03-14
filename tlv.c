#include "tlvFct.h"
#define MSG_HEADER 4
#define TLV_HEADER 2

#define SIZE 1024




int main_datagram(char * msg){
    memset(msg, 0, SIZE);
	uint8_t magic = 0x5d;
	uint8_t ver = 0x2;
	memcpy(msg, &magic, 1);
	memcpy(msg+1, &ver, 1);
	return MSG_HEADER;


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
	memset(neigbour, 0, BUF_SIZE-MSG_HEADER);
	uint8_t type = 0x3;
	//len = 18
	uint8_t len = 0x12;
	memcpy(neigbour, &type, 1);
	memcpy(neigbour+1, &len, 1);
	memcpy(neigbour+2, &IP, 16);
	memcpy(neigbour+18, &port, 2);
	return len+TLV_HEADER;
}


//Creation de Network Hash  (Type de Network Hash???)


//Creation de Network State Request


int Network_state_request(){



}

//Creation de Node Hash


//Creation de Node State Request


//Creation de Node State 


//Creation de warning

int Warning(char * warning, char * message, int length) {
	memset(warning, 0, SIZE-MSG_HEADER);
	uint8_t type = 0x9;
	uint8_t len = length;
	memcpy(warning, &type, 1);
	memcpy(warning+1, &len, 1);
	memcpy(warning+2, message, length);
	return len+TLV_ENTETE;
}



/****************************************************************************/
