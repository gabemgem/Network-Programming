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
#include <utility>
#include <iostream>

char buffer[512];

void connect(std::string comm) {

}

void find_node(std::string comm) {

}

void node(std::string comm) {

}

void find_data(std::string comm) {

}

void value(std::string comm) {

}

void store(std::string comm) {

}

int main(int argc, char* argv[]) {

	int listenfd, connfd, len;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t addrlen = sizeof(cliaddr);
	fd_set rset, allset;
    std::string name = argv[1];
    int port = atoi(argv[2]);
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    std::string line;

    std::vector<std::vector<std::string> > table; 

	bzero(&servaddr, addrlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if(bind(listenfd, (struct sockaddr*)&servaddr, addrlen) == -1) {
		perror("Error binding");
		exit(1);
	}

    if(listen(listenfd, 10)<0) {
    	perror("Error listening");
    	exit(1);
    }

    while(1) {

        FD_ZERO(&allset);
        FD_ZERO(&rset);
        FD_SET(listenfd, &allset);
        FD_SET(0, &allset);

        if (FD_ISSET(listenfd, &rset)) {
            //handle new connections
            bzero(buffer, sizeof(buffer));
            printf("Message from UDP Client: ");
            recvfrom(listenfd, buffer, sizeof(buffer), 0, 
                         (struct sockaddr*)&cliaddr, &addrlen); 

            std::string temp(buffer, 512);
            unsigned int space = temp.find_first_of(' ');
            std::string comm = temp.substr(0, space);

            //Send store client data, send MYID message back
            if (comm == "HELLO") {
                
            }
            //Store MYID data in K-bucket
            else if (comm == "MYID") {

            }
        }

        else if (FD_ISSET(0, &rset)) {
            std::getline(std::cin, line);
            unsigned int space = line.find_first_of(' ');
            std::string comm = line.substr(0, space);  

            if (comm =="CONNECT") {/*LIST COMMAND*/
                        std::cout<<"CONNECT\n";
    					connect(line.substr(space+1));
    				}
            if (comm =="FIND_NODE") {/*LIST COMMAND*/
                        std::cout<<"FIND_NODE\n";
    					find_node(line.substr(space+1));
    				}
            if (comm =="NODE") {/*LIST COMMAND*/
                        std::cout<<"NODE\n";
    					node(line.substr(space+1));
    				}
            if (comm =="FIND_DATA") {/*LIST COMMAND*/
                        std::cout<<"FIND_DATA\n";
    					find_data(line.substr(space+1));
    				}        
            if (comm =="VALUE") {/*LIST COMMAND*/
                        std::cout<<"VALUE\n";
    					value(line.substr(space+1));
    				}    
            if (comm =="STORE") {/*LIST COMMAND*/
                        std::cout<<"STORE\n";
    					store(line.substr(space+1));
    				} 
            if (comm =="QUIT") {/*LIST COMMAND*/
                        std::cout<<"QUIT\n";
    					//quit(line.substr(space+1));
                        break;
    				}             


        }

    }
}



