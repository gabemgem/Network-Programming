#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "packet_handler.h"

packet p;
char* out_packet[PACKETSIZE];
int out_packet_length=0;
twobyte block_num = 0;




void parse_packet(packet* p, const char* pbuffer) {
	char c_op[2];
	memcpy(c_op, pbuffer, sizeof(twobyte));
	p.op = atoi(c_op);

	if(IS_RRQ(p.op) || IS_WRQ(p.op)) {
		int filenamel = 0;
		int model = 0;

		memset(p.filename, 0, sizeof(p.filename));
		memset(p.mode, 0, sizeof(p.mode));

		while(*(pbuffer+sizeof(twobyte)+filenamel)!='0') {
			(p.filename)[filenamel] = pbuffer[sizeof(twobyte)+filenamel];
			filenamel++;
		}
		(p.filename)[filenamel] = ' ';

		while(*(pbuffer+sizeof(twobyte)+filenamel+model+1)!='0') {
			(p.mode)[model] = pbuffer[sizeof(twobyte)+filenamel+model+1];
			model++;
		}
	}

	else if(IS_DAT(p.op)) {
		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);

		p.data_l=0;
		memset(p.data, 0, sizeof(p.data));
		while(*(pbuffer+(2*sizeof(twobyte))+p.data_l)!=NULL) {
			(p.data)[p.data_l] = pbuffer[(2*sizeof(twobyte))+p.data_l];
			p.data_l++;
		}
	}

	else if(IS_ACK(p.op)) {
		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);
	}

	else if(IS_ERR(p.op)) {
		char c_ecode[2];
		memcpy(c_ecode, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.errcode = atoi(c_ecode);

		memset(p.errmes, 0, sizeof(p.errmes));
		p.errmes_l=0;

		while(*(pbuffer+(2*sizeof(twobyte))+p.errmes_l)!='0') {
			(p.errmes)[p.errmes_l] = pbuffer[(2*sizeof(twobyte))+p.errmes_l];
			p.errmes_l++;
		}
		(p.errmes)[p.errmes_l] = ' ';
	}

	else{
		perror("Improper packet received.\n");
	}
}

void packet_handler(packet* p, const char* pbuffer, int* state) {
	parse_packet(p, pbuffer);	

	if(*state==IS_STARTING) {
		handle_request(p, pbuffer);
		if(IS_RRQ(p.op)) {
			*state = IS_WRITING;
		}
		else if(IS_WRQ(p.op)) {
			*state = IS_READING;
		}
		else {
			send_error(ECODE_4);
		}
	}


	else if(*state==IS_WRITING) {
		if(IS_ACK(p.op)) {
			send_data();
		}
		else {
			handle_error();
		}
	}
	else if(*state==IS_READING) {
		if(IS_DAT(p.op)) {
			send_ack();
		}
		else {
			handle_error();
		}
	}
	
}

void make_packet(int type, char* data, int data_l) {
	out_packet = realloc(out_packet, out_packet_length+data_l);
	memmove(out_packet+out_packet_length, data, data_l);
	out_packet_length+=data_l;
}

void make_data(packet* p, char* pbuffer) {
	twobyte op = htons(3);
	

}

