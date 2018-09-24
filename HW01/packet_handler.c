#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "packet_handler.h"

packet p;
transaction t;
char* out_packet[PACKETSIZE];
int out_packet_length=0;
twobyte block_num = 0;




void parse_packet(const char* pbuffer) {
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

void packet_handler(const char* pbuffer) {
	parse_packet(pbuffer);	

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
	
}

void make_packet(char* data, int data_l) {
	out_packet = realloc(out_packet, out_packet_length+data_l);
	memmove(out_packet+out_packet_length, data, data_l);
	out_packet_length+=data_l;
}

void make_data() {
	twobyte op = htons(3);
	twobyte block_num = htons(t.blnum);
	make_packet(&op, sizeof(twobyte));
	make_packet(&block_num, sizeof(twobyte));
	make_packet(&t.filebuffer, t.filebufferl);
}

void make_err() {
	twobyte op = htons(5);
	twobyte errcode = t.errcode;
	make_packet(&op, sizeof(twobyte));
	make_packet(&errcode, sizeof(twobyte));
	make_packet(&t.errmes, t.errmesl);
}

void make_ack() {
	twobyte op = htons(4);
	twobyte block_num = htons(t.blnum);
	make_packet(&op, sizeof(twobyte));
	make_packet(&op, sizeof(twobyte));
}

int receive_rrq(){
        
    if (t.file_open == 1){
        file_close(&t.filedata);
    }
    
    if ((file_open_read(p.filename,&t.filedata))== -1){
        strcpy(t.errmes, ESTRING_1);
        t.errcode = ECODE_1;
        make_error();       
    }else{
        t.file_open = 1;
        t.blnum = 1;
        t.filepos = ((t.blnum * MAXDATA) - MAXDATA);
        t.filebufferl = file_buffer_from_pos(&t);
        make_data();
    }
    
    return 0;
}

int receive_wrq(){
    
    if (t.file_open == 0){ 
    
        if ((file_open_write(p.filename,&t.filedata)) == 0){
            t.file_open = 1;
            
        }else{
            t.file_open = 0;
        }
    }
    
    if (t.file_open == 0){
        strcpy(t.estring, ESTRING_1);
        t.ecode = ECODE_1;
        make_error();       
    }else{
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
            make_error();           
        }
        else{
            make_ack();
            t.blnum++;
        }
    }
    
    if (p.data_length < 512) {
        file_close(&t.filedata);
        t.file_open = 0;
        t.complete = 1;
    }
    
    return 0;
}

int receive_ack(){

    if (p.blnum == t.blnum){
        t.blnum++;
        t.timeout_count = 0;
        
        if (t.file_open == 0 && (file_open_read(p.filename,
                &t.filedata)) == -1){
            strcpy(t.errmes, ESTRING_2);
            t.errcode = ECODE_2;
            make_error();            
        }
        else{
            t.filepos = ((t.blnum * MAXDATA) - MAXDATA);
            t.filebufferl = file_buffer_from_pos(&t);
            if (!t.filebufferl) {
                t.complete = 1;
                return 1;               
            } else {
                make_data();
            }
        }
    }
    
    return 0;
}

int receive_err(){
    perror("Received error %i: %s\n",p.ecode, p.estring);
    return 1;
}

int receive_other(){  
    t.errcode = ECODE_4;
    strcpy(t.errmes, ESTRING_4);
    make_error();
    return 0;
}
