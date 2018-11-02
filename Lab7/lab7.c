#include <netinet/tcp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>



int main(int argc, char** argv) {
	if(argc!=2) {
		printf("Incorrect number of arguments.\n");
		exit(1);
	}

	int sockfd;
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(80);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int rbuffsize, mss;
	unsigned int size = sizeof(int);
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&rbuffsize, &size);
	getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, (char*)&mss, &size);
	printf("Max segment size: %d\n", mss);
	printf("Recieve buffer size: %d\n", rbuffsize);


	connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

	int rbs2, mss2;
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&rbs2, &size);
	getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, (char*)&mss2, &size);
	printf("\nMax segment size: %d\n", mss2);
	printf("Recieve buffer size: %d\n", rbs2);
	close(sockfd);
	exit(0);

}