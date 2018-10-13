#include "../../unpv13e/lib/unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#define STDIN 0

int main(int argc, char **argv)
{
	char buf[MAXLINE];
	int sockfd[5], servers=0;
	struct sockaddr_in servaddr[5];
	char in_port[10];
	fd_set rset;
	FD_ZERO(&rset);
	int servers_ready=0;
	int ports[5];
	char *sendline = "Hello";
	char trash[MAXLINE];
	

	for( ; ; ) {
		
		FD_ZERO(&rset);
		for(int j=0; j<servers; ++j) {
			FD_SET(sockfd[j], &rset);
		}
		FD_SET(STDIN, &rset);
		servers_ready = select(FD_SETSIZE, &rset, NULL, NULL, NULL);
		if(servers_ready<0) {
			perror("Error on select");
		}
		if(FD_ISSET(STDIN, &rset)) {
			servers_ready--;
			if(servers<5) {
				printf("Getting from stdin\n");
				if(fgets(in_port, 10, stdin)!=NULL) {
					printf("Connecting to server on port %d\n", atoi(in_port));
					bzero(&(servaddr[servers]), sizeof(servaddr[servers]));
					servaddr[servers].sin_family = AF_INET;
					servaddr[servers].sin_port = htons(atoi(in_port));
					inet_pton(AF_INET, "127.0.0.1", &(servaddr[servers]).sin_addr);


					sockfd[servers] = socket(AF_INET, SOCK_DGRAM, 0);
					if(sockfd[servers]<0) {
						perror("Error on socket");
					}
					
					socklen_t servaddr_len = sizeof(servaddr[servers]);
					
					sendto(sockfd[servers], sendline, strlen(sendline), 0,(struct sockaddr *) &servaddr[servers], servaddr_len);
					ports[servers] = atoi(in_port);
					

					servers++;
				}
			}
			else {
				fgets(trash, MAXLINE, stdin);
			}
		}
		if(servers_ready>0) {
			for(int i=0; i<servers; ++i) {
				if(FD_ISSET(sockfd[i], &rset)) {
					socklen_t addr_size = sizeof(servaddr[i]);
					if(recvfrom(sockfd[i], buf, MAXLINE, 0, (struct sockaddr *) &servaddr[i], &addr_size)==0) {
						printf("Server on %d has closed\n", ports[i]);
						for(int k=i+1; k<servers; ++k) {
							sockfd[k-1] = sockfd[k];
							ports[k-1] = ports[k];
							servaddr[k-1]=servaddr[k];
						}
						servers--;
					}
					else{
						printf("%d %s", ports[i], buf);
					}
					
				}
			}
		}


	}

	
}
