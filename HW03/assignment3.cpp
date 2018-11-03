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

	if(haspass) {
		printf("%s\n", pass);
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
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


    int num_clients=0;
    std::vector<int> waitingfd;
    std::unordered_map<int, std::string> users;
    std::unordered_map<std::string, std::vector<int> > channels;
    std::vector<int> operators;

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    int nready = 0;
    int readlen = 0;
    char* buffer = (char*)malloc(1024*sizeof(char));

    while(1) {

    	rset = allset;
    	nready = select(FD_SETSIZE, &rset, NULL, NULL, NULL);

    	//If there is a new connection
    	if(FD_ISSET(listenfd, &rset)) {
    		if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &addrlen)) < 0) {
    			perror("Error on accept");
    			exit(EXIT_FAILURE);
    		}
    		printf("Connected to a new person!\n");
    		waitingfd.push_back(connfd);
    		num_clients++;
    		FD_SET(connfd, &allset);
    		nready--;
    		if(nready<=0)
    			continue;
    	}

    	vector<int>::iterator i;
    	vector<int> toDelete;
    	int loc = 0;
    	for(i = waitingfd.begin(); i<waitingfd.end(); i++) {
    		int fd = *i;
    		if(FD_ISSET(fd, &rset)) {
    			memset(buffer, 0, 1024);
    			readlen = read(fd, buffer, 1024);

    			if(readlen==0) {/*USER HAS DISCONNECTED*/
    				FD_CLR(fd, &allset);
    				close(fd);
    			}
    			else {/*PROCESS COMMAND*/
    				std::string temp(buffer, readlen);
    				int space = temp.find_first_of(' ');
    				std::string comm = temp.substr(0, space);
    				if(temp=="USER") {/*VALID COMMAND*/
    					std::string name = temp.substr(space+1);
    					addUser(name, );/****************************************************/
    					
    				}
    				else {/*INVALID COMMAND*/
    					memset(buffer, 0, 1024);
    					strcpy(buffer, "Invalid command, please identify yourself with USER.");
    					send(fd, buffer, strlen(buffer), 0);
    					FD_CLR(fd, &allset);
    					close(fd);
    				}
    			}
    			nready--;
    			if(nready<=0)
    				continue;
    			toDelete.push_back(loc);/*SET FD TO BE DELETED FROM WAITINGFD VECTOR*/
    			loc++;
    		}
    	}

    	if(toDelete.size()>0) {/*DELETE FD FROM WAITINGFD VECTOR*/
	    	i = waitingfd.begin();
	    	advance(i, toDelete[0]);
	    	i = waitingfd.erase(i);
	    	for(int j = 1; j<toDelete.size(); ++j) {
	    		advance(i, toDelete[j]-toDelete[j-1]-1);
	    		i = waitingfd.erase(i);
	    	}
	    }

    	for(std::pair<int, std::string> u : users) {
    		if(FD_ISSET(u.first, &rset)) {
    			memset(buffer, 0, 1024);
    			readlen = read(u.first, buffer, 1024);

    			if(readlen==0) {/*CLIENT HAS DISCONNECTED*/
    				
    			}
    			else {
    				std::string temp;
    				temp = strtok(buffer, " ");

    				if(temp=="LIST") {/*LIST COMMAND*/
    					
    				}
    				else if(temp=="JOIN") {/*JOIN COMMAND*/
    					
    				}
    				else if(temp=="PART") {/*PART COMMAND*/
    					
    				}
    				else if(temp=="OPERATOR") {/*OPERATOR COMMAND*/
    					
    				}
    				else if(temp=="KICK") {/*KICK COMMAND*/
    					
    				}
    				else if(temp=="PRIVMSG") {/*PRIVMSG COMMAND*/
    					
    				}
    				else if(temp=="QUIT") {/*QUIT COMMAND*/
    					
    				}
    				else {/*INVALID ERROR*/
    					
    				}
    			}

    			nready--;
    			if(nready<=0)
    				continue;
    		}
    	}
    }


	return 0;
}