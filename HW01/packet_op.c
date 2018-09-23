#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PACKETSIZE 516
#define MAXDATA 512

typedef struct {
	char filename[PACKETSIZE];
	unsigned short opcode;
	char mode[PACKETSIZE];
	char data[MAXDATA];
	int data_l;
	unsigned short block_num;
	unsigned short error_code;
	char error_mes[PACKETSIZE];
	int error_mes_len;
}packet;

typedef struct {
	unsigned int last_ack;
	int timeout_count;
	int timed_out;
	int bad_packet_count;
	int final_packet;
	int complete;
	int rebound_socket;
	int tid;
	int file_open;
	int filepos;
	int filedata;
	char filebuffer[MAXDATA];
	int filebuffer_length;
	char mode[PACKETSIZE];
	unsigned short block_num;
	unsigned short error_code;
	char error_mes[64];
	int error_mes_len;
}transaction;

char in_packet_buff[PACKETSIZE];
char* in_packet = in_packet_buff;
int in_packet_length;
int out_packet_length;
char* out_packet;
packet p;
transaction t;

void packet_get_opcode(packet* p, const char* buffer) {
	memcpy(&p->opcode, buffer, sizeof(unsigned short));
	p->opcode = ntohs(p->opcode);
}

void packet_do_rq(packet* p, const char* buffer) {
	strcpy(p->filename, buffer+sizeof(unsigned short));
	strcpy(p->mode, buffer+sizeof(unsigned short)+strlen(packet->filename)+1);
}

void packet_do_data(packet* p, const char* buffer, int* data_length) {
	memcpy(&p->block_num, buffer+sizeof(unsigned short), sizeof(unsigned short));
	p->block_num = ntohs(p->block_num);
	p->data_l = *data_length-(2*sizeof(unsigned short));
	memcpy(p->data, buffer+(2*sizeof(unsigned short)), p->data_l);
}

void packet_do_ack(packet* p, const char* buffer) {
	memcpy(&p->block_num, buffer+sizeof(unsigned short), sizeof(unsigned short));
	p->block_num = ntohs(p->block_num);
}

void packet_do_error(packet* p, const char* buffer, int* data_length) {
	memcpy(&p->error_code, pbuf+sizeof(unsigned short), sizeof(unsigned short));
	p->error_code = ntohs(p->error_code);
	p->error_mes_len = *data_length - (2*sizeof(unsigned short));
	memcpy(p->error_mes, buffer+(2*sizeof(unsigned short)), p->error_mes_len);
}

char* make_packet(const void* data, const int data_length) {
	out_packet = realloc(out_packet, sizeof(char)*out_packet_length+data_length);
	memmove(out_packet+out_packet_length, data, data_length);
	out_packet_length+=data_length;
	return out_packet;
}

void make_data(){
	unsigned short op = htons(3);
	unsigned short bl_num = htons(t.block_num);
	make_packet(&op, sizeof(unsigned short));
	make_packet(&bl_num, sizeof(unsigned short));
	make_packet(&t.filebuffer, t.filebuffer_length);
}

void make_error(){
	unsigned short op = htons(5);
	unsigned short ecode = t.error_code;
	make_packet(&op, sizeof(unsigned short));
	make_packet(&ecode, sizeof(unsigned short));
	make_packet(t.error_mes, strlen(t.error_mes)+1);
}

void make_ack(){
	unsigned short op = htons(4);
	printf("\rReceived block: %i", t.block_num);
	unsigned short bl_num = htons(t.block_num);
	make_packet(&op, sizeof(unsigned short));
	make_packet(&bl_num, sizeof(unsigned short));
}

int parse_packet(packet* p, const char* buffer, int* in_packet_length) {
	packet_get_opcode(p, buffer);
	unsigned short op = p->opcode;
	if(op == 1 || op == 2) {
		packet_do_rq(p, buffer);
	}
	else if(op == 4) {
		packet_do_ack(p, buffer);
	}
	else if(op == 3) {
		packet_do_data(p, buffer, in_packet_length);
	}
	else if(op == 5) {
		packet_do_error(p, buffer, in_packet_length);
	}
	else{
		return -1;
	}
	return 0;
}

void free_packet(){
	if(out_packet_length>0) {
		free(out_packet);
		out_packet=NULL;
		out_packet_length=0;
	}
}

int receive_rrq(){
	printf("Read request for %s\n", p.filename);
	free_packet();

	if(t.file_open == 1){
		file_close(&t.filedata);
	}

	if((file_open_read(p.filename, &t.filedata)) == -1){
		strcpy(t.error_mes, "File not found.");
		t.error_code = 1;
		packet_do_error();
	}
	else{
		t.file_open = 1;
		t.block_num = 1;
		t.filepos = ((t.block_num * MAXDATA) - MAXDATA);
		t.filebuffer_length = file_buffer_from_pos(&t);
		packet_do_data();
	}

	return 1;
}

int receive_wrq(){
	printf("Write request for %s\n", p.filename);
	free_packet();

	if(t.file_open == 0){
		if((file_open_write(p.filename, &t.filedata)) == 0){
			t.file_open=1;
		}
		else{
			t.file_open=0;
		}
	}

	if(t.file_open == 0){
		strcpy(t.error_mes, "File not found.");
		t.error_code = 1;
		packet_do_error();
	}
	else{
		packet_do_ack();
		t.block_num++;
	}

	return 1;
}

int receive_data(){
	if(p.block_num == t.block_num){
		if(file_append_from_buffer(&p, &t) == -1){
			strcpy(t.error_mes, "Access violation.");
			t.error_code = 2;
			free_packet();
			packet_do_error();
		}
		else{
			free_packet();
			packet_do_ack();
			t.block_num++;
		}
	}

	if(p.data_length<MAXDATA) {
		file_close(&t.filedata);
		t.file_open = 0;
		t.complete = 1;
	}

	return 1;
}

int receive_ack(){
	if(p.block_num==t.block_num){
		t.block_num++;
		t.timeout_count = 0;
		free_packet();
		if(t.file_open==0 && (file_open_read(p.filename, &t.filedata)) == -1) {
			strcpy(t.error_mes, "Access violation.");
			t.error_code = 2;
			packet_do_error();
		}
		else{
			t.filepos = ((t.block_num*MAXDATA)-MAXDATA);
			t.filebuffer_length = file_buffer_from_pos(&t);
			if(!t.filebuffer_length){
				t.complete=1;
				return OPERATION_ABANDONED;
			}
			else{
				packet_do_data();
			}
		}
	}
	return 1;
}

int receive_err(){
	perror("Received error %i: %s\n", p.error_code, p.error_mes);
	return OPERATION_ABANDONED;
}

int receive_invalid(){
	t.error_code = 4;
	strcpy(t.error_mes, "Illegal TFTP operation.");
	free_packet();
	packet_do_error();
	return 1;
}

