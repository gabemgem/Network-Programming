//#include "unpv13e/lib/unp.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <resolv.h>
#include <strings.h>
#include <netinet/in.h>
#include <signal.h>
#include "packet_handler.h"
#include "file_handler.h"
//#include "unp.h"

#define sendrecvflag 0

extern packet p;
extern transaction t;
extern char* out_packet;
extern int out_packet_length;

void handle_alarm(int sig) {
    t.timeout_count++;
    if(t.timeout_count>=1) {
        t.timed_out=1;
    }
}

/*int file_buffer(transaction_t *transaction) {
  if ((lseek(transaction->filedata, transaction->filepos, SEEK_SET)) == -1){
    printf(stderr, "%s\n", strerror(errno));
    return -1;
  }

  if (int bytes_read = read(transaction->filedata, &transaction->filebuffer, 512) == -1) {
    printf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return bytes_read;
}*/


int main(int argc, char **argv)
{
    signal(SIGALRM, handle_alarm);

    int sockfd, connfd;
    pid_t cpid;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t cliaddr_size = sizeof(cliaddr);

    

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0) {
        perror("Error opening socket");
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = 0;

    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Error binding");
    }

    struct sockaddr_in addr;
    unsigned int myPort;
    char myIP[16];
    bzero(&addr, sizeof(addr));
    int addrlen = sizeof(addr);
    getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
    printf("%s\n", addr.sa_data);
    return 0;

    printf("Waiting for connections");
    //Listen(sockfd, LISTENQ);
    while(1)
    {
        //connfd = Accept(listenfd, (SA*) &cliaddr, sizeof(cliaddr));
        if (!fork())
        {
            while(!t.complete)
            {
                //Area to handle client
                char buf[PACKETSIZE];
                char databuff[0];
                int numbytes;
                numbytes = recvfrom(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
                printf("%d, %s\n", numbytes, buf);

                packet_handler(buf, numbytes);
                //Process Packet for packet obj
                //file_open_read(t.filename, t.filedata);

            }
        }
    }
}


//Request to read or write file, request connection
//If granted access, connection is opened
//File sent in fixed length blocks of 512 bytes
//Data packet of less than 512 bytes signifies transfer opened

//TFTP header consists of a 2 byte opcode field which indicates packet type
