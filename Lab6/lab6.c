#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>


int main(int argc, char** argv) {
	if(argc!=2) {
		printf("Please enter a hostname.\n");
		exit(0);
	}

	int err;
	struct addrinfo* servinfo;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if((err=getaddrinfo(argv[1], NULL, &hints, &servinfo)) !=0) {
		fprintf(stderr, "getaddrinfo error code: %s\n", gai_strerror(err));
		perror("getaddrinfo perror: ");
		exit(1);
	}

	if(servinfo!=NULL) {
		char* print_address = (char*)malloc(INET6_ADDRSTRLEN*sizeof(char));
		
		inet_ntop(servinfo->ai_family, &(((struct sockaddr_in*)(servinfo->ai_addr))->sin_addr), print_address, INET6_ADDRSTRLEN);
		printf("%s\n", print_address);

		while(servinfo->ai_next!=NULL){
			servinfo = servinfo->ai_next;
			memset(print_address, 0, strlen(print_address));
			inet_ntop(servinfo->ai_family, &(((struct sockaddr_in*)(servinfo->ai_addr))->sin_addr), print_address, INET6_ADDRSTRLEN);
			printf("%s\n", print_address);
			
		}
		free(print_address);
	}
	
	freeaddrinfo(servinfo);
	return 0;
}