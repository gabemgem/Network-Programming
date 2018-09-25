#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "packet_handler.h"
#include "file_handler.h"

packet p;
transaction t;
char* out_packet;
int out_packet_length=0;
char in_packet[PACKETSIZE];
int in_packet_length;



void parse_packet(const char* pbuffer, const int buff_size) {
	
	memmove(&p.op, pbuffer, sizeof(twobyte));
	p.op = ntohs(p.op);
	
	printf("Finished parsing op code\n");
	printf("OP: %o\n", p.op);

	if(IS_RRQ(p.op) || IS_WRQ(p.op)) {
		/*int filenamel = 0;
		int model = 0;

		memset(p.filename, 0, sizeof(p.filename));
		memset(p.mode, 0, sizeof(p.mode));

		while(*(pbuffer+sizeof(twobyte)+filenamel)!='0') {
			(p.filename)[filenamel] = pbuffer[sizeof(twobyte)+filenamel];
			filenamel++;
		}
		(p.filename)[filenamel] = ' ';
		*/
		strcpy(p.filename, pbuffer+sizeof(twobyte));
		printf("Finished getting filename into packet\n");

		/*
		while(*(pbuffer+sizeof(twobyte)+filenamel+model+1)!='0') {
			(p.mode)[model] = pbuffer[sizeof(twobyte)+filenamel+model+1];
			model++;
		}
		*/
		strcpy(p.mode, pbuffer+sizeof(twobyte)+strlen(p.filename)+1);
		printf("Finished getting mode into packet\n");
	}

	else if(IS_DAT(p.op)) {
		memcpy(&p.blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = ntohs(p.blnum);
		printf("Finished getting block num into packet: %u\n", p.blnum);
		/*
		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);
		*/
		p.data_l = buff_size-(2*sizeof(twobyte));
		memcpy(p.data, pbuffer+(2*sizeof(twobyte)), p.data_l);
		printf("Finished getting data into packet\n");
		/*
		p.data_l=0;
		memset(p.data, 0, sizeof(p.data));
		while(p.data_l<buff_size-(2*sizeof(twobyte))) {
			(p.data)[p.data_l] = pbuffer[(2*sizeof(twobyte))+p.data_l];
			p.data_l++;
		}
		*/
	}

	else if(IS_ACK(p.op)) {
		memcpy(&p.blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = ntohs(p.blnum);
		/*
		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);
		*/
	}

	else if(IS_ERR(p.op)) {
		memcpy(&p.errcode, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.errcode = ntohs(p.errcode);
		p.errmes_l = buff_size - (2*sizeof(twobyte));
		memcpy(p.errmes, pbuffer+(2*sizeof(twobyte)), p.errmes_l);
		/*
		char c_ecode[2];
		memcpy(c_ecode, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.errcode = atoi(c_ecode);

		memset(p.errmes, 0, sizeof(p.errmes));
		int errmes_l=0;

		while(*(pbuffer+(2*sizeof(twobyte))+errmes_l)!='0') {
			(p.errmes)[errmes_l] = pbuffer[(2*sizeof(twobyte))+errmes_l];
			errmes_l++;
		}
		(p.errmes)[errmes_l] = ' ';
		*/
	}

	else{
		perror("Improper packet received.\n");
	}
}

void packet_handler(const char* pbuffer, const int buff_size) {
	t.packet_ready = 0;
	parse_packet(pbuffer, buff_size);
	printf("Finished parsing packet\n");

	if(IS_RRQ(p.op)) {
		receive_rrq();
	}
	else if(IS_WRQ(p.op)) {
		receive_wrq();
	}
	else if(IS_DAT(p.op)) {
		receive_data();
	}
	else if(IS_ACK(p.op)) {
		receive_ack();
	}
	else if(IS_ERR(p.op)) {
		receive_err();
	}
	else {
		receive_other();
	}
	printf("Out packet ready\n");
	printf("Block num: %u\n", t.blnum-1);

}

void make_packet(void* data, int data_l) {
	out_packet = (char*)realloc(out_packet, out_packet_length+data_l);
	if(out_packet==NULL) {
		perror("Could not realloc out_packet");
		exit(1);
	}

	memmove(out_packet+out_packet_length, data, data_l);
	out_packet_length+=data_l;
}

void make_data() {
	twobyte op = htons(3);
	twobyte block_num = htons(t.blnum);
	make_packet(&op, sizeof(twobyte));
	make_packet(&block_num, sizeof(twobyte));
	make_packet(&t.filebuffer, t.filebufferl);
	t.packet_ready = 1;
}

void make_ack() {
	twobyte op = htons(4);
	twobyte block_num = htons(t.blnum);
	make_packet(&op, sizeof(twobyte));
	make_packet(&block_num, sizeof(twobyte));
	t.packet_ready = 1;
}

void make_err() {
	twobyte op = htons(5);
	twobyte errcode = t.errcode;
	make_packet(&op, sizeof(twobyte));
	make_packet(&errcode, sizeof(twobyte));
	make_packet(&t.errmes, strlen(t.errmes));
	t.packet_ready = 1;
}

void clear_out_packet() {
	if(out_packet_length>0) {
		free(out_packet);
		out_packet_length=0;
		out_packet = NULL;
	}
}

int receive_rrq(){

	clear_out_packet();

    if (t.file_open == 1){
        file_close(&t.filedata);
    }

    if ((file_open_read(p.filename,&t.filedata))== -1){
        strcpy(t.errmes, ESTRING_1);
        t.errcode = ECODE_1;

        make_err();
    }
    else{
        t.file_open = 1;
        t.blnum = 1;
        t.filepos = ((t.blnum * MAXDATA) - MAXDATA);
        t.filebufferl = file_buffer_from_pos(&t);
        make_data();
        t.blnum++;
        
    }

    return 0;
}

int receive_wrq(){

    clear_out_packet();

    if (t.file_open == 0){

        if ((file_open_write(p.filename,&t.filedata)) == 0){
            t.file_open = 1;

        }else{
            t.file_open = 0;
        }
    }

    if (t.file_open == 0){
        strcpy(t.errmes, ESTRING_1);
        t.errcode = ECODE_1;
        make_err();
    }else{
    	t.blnum = 0;
        make_ack();
        
        t.blnum++;
    }

    return 0;
}

int receive_data(){

    if (p.blnum == t.blnum){
    	
        if (file_append_from_buffer(&p, &t) == -1){
            strcpy(t.errmes, ESTRING_2);
            t.errcode = ECODE_2;
            clear_out_packet();
            make_err();
        }
        else{
        	clear_out_packet();
            make_ack();
            t.blnum++;
            
        }
    }

    if (p.data_l < 512) {
        file_close(&t.filedata);
        t.file_open = 0;
        t.complete = 1;
        printf("Completing receive\n");
    }

    return 0;
}

int receive_ack(){

    if (p.blnum == t.blnum){
        t.blnum++;
        clear_out_packet();
        

        if (t.file_open == 0 && (file_open_read(p.filename,
                &t.filedata)) == -1){
            strcpy(t.errmes, ESTRING_2);
            t.errcode = ECODE_2;
            make_err();
        }
        else{
            t.filepos = ((t.blnum * MAXDATA) - MAXDATA);
            t.filebufferl = file_buffer_from_pos(&t);
            if (!t.filebufferl) {
                t.complete = 1;
                printf("Completing send\n");
            } else {
                make_data();
            }
        }
    }

    return 0;
}

int receive_err(){
    fprintf(stderr, "Received error %i: %s\n",p.errcode, p.errmes);
    return 1;
}

int receive_other(){
    t.errcode = ECODE_4;
    strcpy(t.errmes, ESTRING_4);
    make_err();
    return 0;
}
