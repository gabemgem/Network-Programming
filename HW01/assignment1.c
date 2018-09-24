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
char buf[PACKETSIZE];

void handle_alarm(int sig) {
    t.timed_out=1;
}



int main(int argc, char **argv)
{
    signal(SIGALRM, handle_alarm);
    int sockfd;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t cliaddr_size = sizeof(cliaddr);
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
        char buf[PACKETSIZE];
        int numbytes;
        numbytes = recvfrom(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
        printf("Connection has been made\n");
        
        if (!fork())
        {
            int received = 0;
            fd_set readfds, masterfds;
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            FD_ZERO(&masterfds);
            FD_SET(sockfd, &masterfds);

            memcpy(&readfds, &masterfds, sizeof(fd_set));

            printf("%s\n", buf);
            sendto(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
            printf("Process has been forked!\n");
            servaddr.sin_port = rand()%65535;
            if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
                perror("Error on second bind");
                exit(1);
            }
            

            while(!t.complete)
            {
                //Area to handle client
                //Process Packet for packet obj
                //file_open_read(t.filename, t.filedata

                packet_handler(buf, numbytes);
                while(!t.packet_ready);
                sendto(sockfd, out_packet, out_packet_length, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
                printf("Packet: %s", buf);
                received = 0;
                t.timed_out = 0;
                alarm(10);
                
                while(!received) {
                    if(t.timed_out==1) {
                        perror("Timed out");
                        close(sockfd);
                        return 0;
                    }

                    if(select(sockfd+1, &readfds, NULL, NULL, &timeout)<0) {
                        perror("Error on select");
                        exit(1);
                    }

                    if(FD_ISSET(sockfd, &readfds)) {
                        numbytes = recvfrom(sockfd, buf, PACKETSIZE, sendrecvflag, (struct sockaddr *) &cliaddr, &cliaddr_size);
                        received=1;
                    }
                    
                    else {
                        sendto(sockfd, out_packet, out_packet_length, sendrecvflag, (struct sockaddr *) &cliaddr, cliaddr_size);
                    }
                }
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
