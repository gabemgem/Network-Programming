#include "../unpv13e/lib/unp.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sockfd, connfd;
    pid_t cpid;
    struct sockaddr_in cliaddr, servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(0);

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if(Bind(sockfd, (SA *) &servaddr, sizeof(servaddr))==-1) {
    	perror("Bind failed\n");
    	exit(1);
    }
    printf("%d\n", servaddr.sin_port);
    Listen(sockfd, LISTENQ);
    while(1)
    {
        connfd = Accept(listenfd, (SA *) &cliaddr, sizeof(cliaddr));
        if(connfd == -1) {
        	perror("accept");
        	continue;
        }



        cpid = Fork();
        if (cpid == 0)
        {
            close(sockfd);
            char buff[512];
            int numBytes, ack_bytes;
            header_t head;
            packet_t packet;
            packet_t ack;
            char *filename = (char *) "recvfile.txt";

            FILE *file;
            bzero(buff, 512);
            int done = 1;

            while(!done)
            {
            	if((numBytes = recv(conn_fd, buff, 512, 0)) == -1) {
            		perror("recv");
            		fclose(file);
            		close(conn_fd);
            		exit(1);
            	}


            }
        }
    }



}

//Request to read or write file, request connection
//If granted access, connection is opened
//File sent in fixed length blocks of 512 bytes
//Data packet of less than 512 bytes signifies transfer opened

//TFTP header consists of a 2 byte opcode field which indicates packet type
