#include "../../unpv13e/lib/unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	if(argc!=2) {
		printf("Incorrect number of args.\n");
		exit(1);
	}
	int port = 9877 + atoi(argv[1]);

	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))==-1) {
		perror("Error binding");
		exit(1);
	}

	socklen_t cliaddr_size = sizeof(cliaddr);
	char in[20];
	recvfrom(sockfd, in, 20, 0, (struct sockaddr *) &cliaddr, &cliaddr_size);
	printf("Connection has been made.\n");


	for( ; ; ) {
		char str[MAXLINE];
		if(fgets(str, MAXLINE, stdin)!=NULL) {
			printf("Sending:\n%s : %lu\n", str, strlen(str));
			if(sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&cliaddr, cliaddr_size)<0) {
				perror("Error on send");
			}
		}
		else {
			printf("Exiting\n");
			char end[0];
			sendto(sockfd, end, 0, 0, (struct sockaddr *)&cliaddr, cliaddr_size);
			close(sockfd);
			return 0;
		}
			
			
		
	}
	return 0;
}