#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
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
transaction t;

#define sendrecvflag 0
extern packet p;
extern transaction t;
extern char* out_packet;
extern int out_packet_length;
char buf[PACKETSIZE];

void handle_alarm(int sig) {
    t.timeout_count++;
    if(t.timeout_count>=1) {
        t.timed_out=1;

    }
}



int main(int argc, char **argv)
{
    signal(SIGALRM, handle_alarm);
    int sockfd, connfd;
    pid_t cpid;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t cliaddr_size = sizeof(cliaddr);
    printf("Hello World\n");
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = 0;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0) {
        perror("Error opening socket");
        exit(1);
    }
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1){
      perror("bind");
      exit(1);
    }

    struct sockaddr_in addr;
    unsigned int myPort;
    char myIP[16];
    bzero(&addr, sizeof(addr));
    int addrlen = sizeof(addr);
    getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
    printf("%s\n", addr.sa_data);
    return 0;
    
    printf("Waiting for connections\n");

    while(1)
    {
        char buf[PACKETSIZE];
        int numbytes;
        numbytes = recvfrom(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
        printf("Connection has been made\n");
        printf("%s\n", buf);
        sendto(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
        if (!fork())
        {
            printf("Process has been forked!\n");
            servaddr.sin_port = rand()%65535;
            bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("%d\n", servaddr.sin_port);
            printf("%d, %s\n", numbytes, buf);
            packet_handler(buf, numbytes);
            //sendto(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);

            while(!t.complete)
            {
                //Area to handle client
                //Process Packet for packet obj
                //file_open_read(t.filename, t.filedata

                sendto(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
                printf("Packet: %s", buf);
                alarm(10);

                numbytes = recvfrom(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
                printf("Packet: %s", buf);
                packet_handler(buf, numbytes);



            }
            close(sockfd);
            return 0;
        }
    }
    return 0;
}


/*
Server listens on random socket port
Client sends request
fork
Server sends udp with new source and destination ports
Client attaches to new port



*/
