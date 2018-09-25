#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <resolv.h>
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
extern char in_packet[PACKETSIZE];
extern int in_packet_length;

void handle_alarm(int sig) {
    t.timed_out=1;
}



int main(int argc, char **argv)
{
    signal(SIGALRM, handle_alarm);
    int sockfd;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t cliaddr_size = sizeof(cliaddr);
    
    


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0) {
        perror("Error opening socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = 0;
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1){
      perror("Error binding");
      exit(1);
    }

    struct sockaddr_in addr;
    unsigned int myPort;
    bzero(&addr, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
    myPort = ntohs(addr.sin_port);
    printf("%u\n", myPort);

    printf("Waiting for connections\n");

    while(1)
    {
        
        in_packet_length = recvfrom(sockfd, in_packet, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
        printf("Connection has been made\n");
        
        if (!fork())
        {
            int pid = getpid();
            srand((unsigned int)pid);

            t.timed_out = 0;
            t.final = 0;
            t.complete = 0;
            t.file_open = 0;
            t.filepos = 0;
            t.filedata = 0;
            t.filebufferl = 0;
            t.blnum = 0;
            t.errcode = 0;
            t.packet_ready = 0;

            printf("Process has been forked!\n");

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(sockfd<0) {
                perror("Error opening socket 2");
                exit(1);
            }

            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            servaddr.sin_port = htons(rand()%65535);
            if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
                perror("Error on second bind");
                exit(1);
            }

            struct sockaddr_in addr;
            unsigned int myPort;
            bzero(&addr, sizeof(addr));
            socklen_t addrlen = sizeof(addr);
            getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
            myPort = ntohs(addr.sin_port);
            printf("Rebound to port %u\n", myPort);

            int received = 0;
            fd_set readfds, masterfds;
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            FD_ZERO(&masterfds);
            FD_SET(sockfd, &masterfds);

            memcpy(&readfds, &masterfds, sizeof(fd_set));


            while(!t.complete)
            {
                //Area to handle client
                //Process Packet for packet obj
                //file_open_read(t.filename, t.filedata
                packet_handler(in_packet, in_packet_length);
                
                sendto(sockfd, out_packet, out_packet_length, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
                printf("Out packet sent\n");

                received = 0;
                t.timed_out = 0;
                alarm(10);
                
                while(!received && !t.complete) {
                    if(t.timed_out==1) {
                        perror("Timed out1");
                        close(sockfd);
                        return 0;
                    }

                    if(select(sockfd+1, &readfds, NULL, NULL, &timeout)<0) {
                        perror("Error on select");
                        exit(1);
                    }

                    if(FD_ISSET(sockfd, &readfds)) {
                        in_packet_length = recvfrom(sockfd, in_packet, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
                        received=1;
                        printf("Reply received\n");
                    }
                    
                    else {
                        sendto(sockfd, out_packet, out_packet_length, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
                        printf("Out packet resent\n");
                    }
                }
                printf("\n\n");



            }
            close(sockfd);
            printf("Closed the socket\n");
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
