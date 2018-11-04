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

char buffer[512];

void addUser(	int fd
				, std::string name
				,std::unordered_map<int, std::string> &users
				) {

	//ADD CHECKNAME


	std::pair<int, std::string> userPair(int, name);
	users.insert(userPair);
	sprintf(buffer, "Welcome, %s", name.c_str());
	send(fd, buffer, strlen(buffer), 0);


	
	

}
void sendToChannel(	std::string message
					, vector<int> channel
					) {
	

}

void invalidCommand(int fd
					) {
	char* message = "Invalid command.";
	send(fd, message, strlen(message), 0);

}

void quit(	int fd
			, std::unordered_map<std::string, std::vector<int> > channels
			, std::unordered_map<int, std::string> users
			, std::vector<int> operators
			) {

}

void list(	int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int> > channels
			, std::unordered_map<int, std::string> users
			) {
	
	std::unordered_map<int, std::string>::iterator itr;
	std::vector<int> userlist = channels[comm];
	std::string message = "Users in channel "+com+"\n";
	send(fd, message.c_str(), message.size(), 0);

	for (unsigned int i = 0; i < userlist.size(); i++) {
		send(fd, userlist[i].c_str(), userlist[i].size(), 0);
	}



}

void join(int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int> > channels
			, std::unordered_map<int, std::string> users
			) {
	if (channels.find(comm) != channels.end())
	{
		std::vector<int> v;
		v.push_back(fd);
		std::pair<std::string, std::vector<int> > channelPair(comm, v);
		channels.insert(channelPair);
	}
	else {
		channels[comm].push(fd);
	}
	
}

void part(int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int> > channels
			, std::unordered_map<int, std::string> users
			) {
	std::unordered_map<int, std::string>::iterator itr = channels.find(comm);
	if (itr != channels.end()) {
		//Channel Exists
		std::vector<int> v = itr->second;		
		std::vector<int>::iterator itr;
		for (itr = v.begin(); itr != v.end(); itr++) {
			
		}

	}

	else {
		//Print channel does not exist

	}
	
	
}

void op(int fd
		, std::string pass
		, std::vector<int> operators
		) {

}

void kick(	int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int> > channels
			, std::unordered_map<int, std::string> users
			, std::vector<int> operators
			) {}

}

void privmsg(	int fd
				, std::string comm
				, std::unordered_map<std::string, std::vector<int> > channels
				, std::unordered_map<int, std::string> users
				) {

}



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
    				if(space==string::npos) {/*NO SPACES FOUND*/
    					invalidCommand(fd);
    					FD_CLR(fd, &allset);
    					close(fd);
    				}
    				else if(comm=="USER") {/*VALID COMMAND*/
    					std::string name = temp.substr(space+1);
    					addUser(fd, name, );/****************************************************/
    					
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
    		int fd = u.first;
    		if(FD_ISSET(fd, &rset)) {
    			memset(buffer, 0, 1024);
    			readlen = read(fd, buffer, 1024);

    			if(readlen==0) {/*CLIENT HAS DISCONNECTED*/
    				quit(fd, channels, users, operators);/**************************************************************/
    			}
    			else {
    				std::string buff(buffer, readlen);
    				int space = buff.find_first_of(' ');
    				std::string temp = buff.substr(0, space);
    				if(space==string::npos) {
    					invalidCommand(fd);
    				}
    				else if(temp=="LIST") {/*LIST COMMAND*/
    					list(fd, buff.substr(space+1), channels, users);/*************************************/
    				}
    				else if(temp=="JOIN") {/*JOIN COMMAND*/
    					join(fd, buff.substr(space+1), channels, users);/*************************************/
    				}
    				else if(temp=="PART") {/*PART COMMAND*/
    					part(fd, buff.substr(space+1), channels, users);/*************************************/
    				}
    				else if(temp=="OPERATOR") {/*OPERATOR COMMAND*/
    					op(fd, buff.substr(space+1), operators);/***************************************/
    				}
    				else if(temp=="KICK") {/*KICK COMMAND*/
    					kick(fd, buff.substr(space+1), channels, users, operators);/*************************************/
    				}
    				else if(temp=="PRIVMSG") {/*PRIVMSG COMMAND*/
    					privmsg(fd, buff.substr(space+1), channels, users);/**********************************/
    				}
    				else if(temp=="QUIT") {/*QUIT COMMAND*/
    					quit(fd, );/***********************************************************/
    				}
    				else {/*INVALID ERROR*/
    					invalidCommand(fd);
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