#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PACKETSIZE 516
#define MAXDATA 512

#define IS_STARTING 0
#define IS_READING 1
#define IS_WRITING 2


#define OP_RRQ 1
#define OP_WRQ 2
#define OP_DAT 3
#define OP_ACK 4
#define OP_ERR 5
#define IS_RRQ(op) ((op)==OP_RRQ)
#define IS_WRQ(op) ((op)==OP_WRQ)
#define IS_DAT(op) ((op)==OP_DAT)
#define IS_ACK(op) ((op)==OP_ACK)
#define IS_ERR(op) ((op)==OP_ERR)


#define ECODE_NONE  8
#define ECODE_0     0
#define IS_ECODE_0(ec)  ((ec) == ECODE_0)
#define ESTRING_0   "Not defined, see error message(if any)."
#define ECODE_1     1
#define IS_ECODE_1(ec)  ((ec) == ECODE_1)
#define ESTRING_1   "File not found."
#define ECODE_2     2
#define IS_ECODE_2(ec)  ((ec) == ECODE_2)
#define ESTRING_2   "Access violation."
#define ECODE_3     3
#define IS_ECODE_3(ec)  ((ec) == ECODE_3)
#define ESTRING_3   "Disk full or allocation exceeded."
#define ECODE_4     4
#define IS_ECODE_4(ec)  ((ec) == ECODE_4)
#define ESTRING_4   "Illegal TFTP operation."
#define ECODE_5     5
#define IS_ECODE_5(ec)  ((ec) == ECODE_5)
#define ESTRING_5   "Unknown transfer ID."
#define ECODE_6     6
#define IS_ECODE_6(ec)  ((ec) == ECODE_6)
#define ECODE_7     7
#define IS_ECODE_7(ec)  ((ec) == ECODE_7)
#define ESTRING_7   "No such user."


typedef unsigned short twobyte;

typedef struct {
	twobyte op;
	char filename[PACKETSIZE];
	char mode[PACKETSIZE];
	char data[MAXDATA];
	int data_l;
	twobyte blnum;
	twobyte errcode;
	char errmes[PACKETSIZE];
	int errmes_l;
}packet;



int main(int argc, char** argv){
	if(argc!=2)
		return 1;

	int test = atoi(argv[1]);
	packet p;
	

	if(IS_RRQ(test) || IS_WRQ(test)) {
		char* pbuffer = "01filename.txt0octal0";
		char c_op[2];
		memcpy(c_op, pbuffer, sizeof(twobyte));
		p.op = atoi(c_op);
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
		printf("%d\n%s\n%s\n", p.op, p.filename, p.mode);
	}

	else if(IS_DAT(test)) {
		char* pbuffer = "0301ThisIsSomeData:asdfjkl;;lkjfdsa";
		char c_op[2];
		memcpy(c_op, pbuffer, sizeof(twobyte));
		p.op = atoi(c_op);

		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);

		p.data_l=0;
		memset(p.data, 0, sizeof(p.data));
		while(*(pbuffer+(2*sizeof(twobyte))+p.data_l)!=NULL) {
			(p.data)[p.data_l] = pbuffer[(2*sizeof(twobyte))+p.data_l];
			p.data_l++;
		}

		printf("%d\n%d\n%s\n%d\n", p.op, p.blnum, p.data, p.data_l);
	}

	else if(IS_ACK(test)) {
		char* pbuffer = "0401";
		char c_op[2];
		memcpy(c_op, pbuffer, sizeof(twobyte));
		p.op = atoi(c_op);

		char c_blnum[2];
		memcpy(c_blnum, pbuffer+sizeof(twobyte), sizeof(twobyte));
		p.blnum = atoi(c_blnum);

		printf("%d\n%d\n", p.op, p.blnum);
	}

	else if(IS_ERR(test)) {
		char* pbuffer = "0501File not found.0";
		char c_op[2];
		memcpy(c_op, pbuffer, sizeof(twobyte));
		p.op = atoi(c_op);

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

		printf("%d\n%d\n%s\n%d\n", p.op, p.errcode, p.errmes, p.errmes_l);
	}

	else {
		perror("Improper packet received.\n");
	}



	/*
	*/

	
	return 0;
}