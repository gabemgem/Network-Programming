
#ifndef TFTP
#define TFTP

#define PACKETSIZE 516
#define MAXDATA 512


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
}packet;

typedef struct {
	int timeout_count;
	int timed_out;
	int final;
	int complete;
	int tid;
	int file_open;
	int filepos;
	int filedata;
	char filebuffer[MAXDATA];
	int filebufferl;
	char mode[PACKETSIZE];
	twobyte blnum;
	twobyte errcode;
	char errmes[64];
}transaction;

void parse_packet(const char* pbuffer);
void packet_handler(const char* pbuffer);
void make_packet(void* data, int data_l);
void make_data();
void make_err();
void make_ack();
int receive_rrq();
int receive_wrq();
int receive_data();
int receive_ack();
int receive_err();
int receive_other();

#endif