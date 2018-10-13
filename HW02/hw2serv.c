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

#define MAXLINE 1024

int main(int argc, char** argv) {
	int err, listenfd, maxi, maxfd, connfd, sockfd;
	int nready, num_clients = 0, client[FD_SETSIZE];
	ssize_t n;

	fd_set rset, allset;

	char in_buffer[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	//Opens the socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0) {
		perror("Error opening socket");
		exit(1);
	}

	//Binds to a port on localhost that is assigned by the system
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl("127.0.0.1");
	servaddr.sin_port = 0;
	if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1){
		perror("Error binding");
		exit(1);
	}

	//Gets own port number and prints it
	struct sockaddr_in addr;
    unsigned int myPort;
    bzero(&addr, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
    myPort = ntohs(addr.sin_port);
    printf("%u\n", myPort);

    //Listens on bound socket, max 5 pending connections
    if(listen(listenfd, 5)<0) {
    	perror("Error listening");
    	exit(1);
    }

    //Initialize client array and select set
    maxfd = listenfd;
    maxi = -1;
    for(int i=0; i<FD_SETSIZE; ++i) {
    	client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);



    for( ; ; ) {
    	rset = allset;
    	nready = select(maxfd+1, &rset, NULL, NULL, NULL);

    	//if there is a new client
    	if(FD_ISSET(listenfd, &rset) && num_clients<5) {
    		clilen = sizeof(cliaddr);
    		if(connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)<0) {
    			perror("Error on accept");
    		}

    		printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));

    		for(int i=0; i<FD_SETSIZE; ++i) {
    			if(client[i]<0) {
    				client[i] = connfd;
    				break;
    			}
    		}
    		num_clients++;
    		
    		FD_SET(connfd, &allset);
    		if(connfd>maxfd)
    			maxfd = connfd;
    		if(num_clients-1>maxi) 
    			maxi = num_clients-1;
    		if(--nready <=0)
    			continue;
    	}

    	for(int i=0; i<=maxi; ++i) {
    		if((sockfd = client[i]) < 0)
    			continue;
    		if(FD_ISSET(sockfd, &rset)) {
    			if((n=read(sockfd, in_buffer, MAXLINE)) == 0) {
    				close(sockfd);
    				FD_CLR(sockfd, &allset);
    				client[i] = -1;
    			}
    			else {
    				printf("Now is when I do stuff on string:\n%s\n", in_buffer);
    			}
    			if(--nready <=0)
    				break;
    		}
    	}
    }

}