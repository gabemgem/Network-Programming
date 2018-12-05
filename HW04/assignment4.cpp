#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <resolv.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

//cpp exclusive
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <cstring>
#include <utility>
#include <iostream>

char buffer[512];

struct threeTuple {
    std::string name;
    int port;
    char id;
};

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

int dist(int a, int b) {
    return a^b;
}

int main(int argc, char* argv[]) {

    if(argc!=5) {
        printf("Wrong number of args.\n");
        return 0;
    }

	int connfd, len;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t addrlen = sizeof(cliaddr);
	fd_set rset, allset;
    std::string name = argv[1];
    int port = atoi(argv[2]);
    connfd = socket(AF_INET, SOCK_DGRAM, 0);
    std::string line;
    int id = atoi(argv[3]);

    std::vector<std::list<threeTuple> > table(9); 

	bzero(&servaddr, addrlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if(bind(connfd, (struct sockaddr*)&servaddr, addrlen) == -1) {
		perror("Error binding");
		exit(1);
	}

    std::string myid = "MYID "+id;

    FD_ZERO(&allset);
    FD_ZERO(&rset);
    FD_SET(connfd, &allset);
    FD_SET(0, &allset);
    int nready=0;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    while(1) {
        rset = allset;
        if(tv.tv_sec==0 && tv.tv_usec==0) {
            tv.tv_sec=3;
            tv.tv_usec=0;
        }
        nready = select(FD_SETSIZE, &rset, NULL, NULL, &tv);
        
        
        
        

        if (FD_ISSET(connfd, &rset)) {
            //handle incoming messages
            bzero(buffer, 512);
            printf("Message from UDP Client: \n");
            recvfrom(connfd, buffer, 512, 0, 
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
            nready--;
        }

        else if (nready>0 && FD_ISSET(0, &rset)) {//handle incoming commands
            std::getline(std::cin, line);
            std::cout<<line<<"\n";
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



