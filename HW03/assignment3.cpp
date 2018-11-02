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
#include <time.h>
#include <ctype.h>

//cpp exclusive
#include <vector>
#include <string>
#include <unordered_map>
#include <cstring>


int main(int argc, char* argv[]) {

	std::string pass="";
	bool haspass = false;
	if(argc>1) {
		std::string temp;
		temp = strtok(argv[1], "=");
		if(temp == "--opt-pass") {
			pass = strtok(NULL, "=");
			haspass=true;
		}
	}


	int listenfd, connfd;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t addrlen = sizeof(cliaddr);
	fd_set rset, allset;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0) {
		perror("Error opening socket");
		exit(1);
	}

	bzero(&servaddr, addrlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("localhost");//htonl(INADDR_ANY);
	servaddr.sin_port = 0;
	if(bind(listenfd, (struct sockaddr*)&servaddr, addrlen) == -1) {
		perror("Error binding");
		exit(1);
	}

	struct sockaddr_in tempaddr;
    unsigned int myPort;
    bzero(&tempaddr, sizeof(tempaddr));
    getsockname(listenfd, (struct sockaddr *) &tempaddr, &addrlen);
    myPort = ntohs(tempaddr.sin_port);
    printf("%u\n", myPort);

    if(listen(listenfd, 10)<0) {
    	perror("Error listening");
    	exit(1);
    } 


    int num_clients;
    std::vector<int> waitingfd;
    std::unordered_map<std::string, int> users;
    std::unordered_map<std::string, std::vector<std::string> > channels;
    std::vector<std::string> operators;


	return 0;
}