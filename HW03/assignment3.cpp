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

void addUser(	int fd
				, std::string name
				,std::unordered_map<int, std::string> *users
				) {

    if(name.find_first_of('#')!=std::string::npos) {	//ADD CHECKNAME
        return;
    }
	std::pair<int, std::string> userPair(fd, name);
	(*users).insert(userPair);
	sprintf(buffer, "Welcome, %s\n", name.c_str());
	send(fd, buffer, strlen(buffer), 0);
}

void sendToChannel(	std::string message
					, std::vector<int> channel
					) {
	for (unsigned int i = 0; i < channel.size(); i++) {
		send(channel[i], message.c_str(), strlen(message.c_str()), 0);
	}

}

void invalidCommand(int fd
					) {
	std::string message = "Invalid command.\n";
	send(fd, message.c_str(), strlen(message.c_str()), 0);

}

void quit(	int fd
			, std::unordered_map<std::string, std::vector<int>* >* channels
			, std::unordered_map<int, std::string>* users
			, std::vector<int>* operators
			) {
    std::unordered_map<std::string, std::vector<int>* >::iterator channel_it = (*channels).begin();
    while(channel_it!=(*channels).end()) {
        std::vector<int>::iterator chit = (*(channel_it->second)).begin();
        while(chit!=(*(channel_it->second)).end()) {
            if(*chit==fd)
                chit = (*(channel_it->second)).erase(chit);
            else
                chit++;
        }
    }

    std::unordered_map<int, std::string>::iterator users_it = (*users).begin();
    while(users_it!=(*users).end()) {
        if(users_it->first==fd)
            users_it = (*users).erase(users_it);
        else
            users_it++;
    }

    std::vector<int>::iterator op_it = (*operators).begin();
    while(op_it!=(*operators).end()) {
        if(*op_it==fd)
            op_it = (*operators).erase(op_it);
        else
            op_it++;
    }

}

void list(	int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int>* >* channels
			, std::unordered_map<int, std::string>* users
			) {

    std::unordered_map<std::string, std::vector<int>* >::iterator itr;
	if(comm.size()==0 || comm[0]!='#') {
        std::string message = "There are currently "+std::to_string((*channels).size())+" channels.\n";
        send(fd, message.c_str(), strlen(message.c_str()), 0);
        for(itr = (*channels).begin(); itr!=(*channels).end(); itr++) {
            message = "* " +itr->first.substr(1)+"\n";
            send(fd, message.c_str(), strlen(message.c_str()), 0);
        }
        return;
    }

	itr = (*channels).find(comm);
    if(itr == (*channels).end()) {
        std::string message = "There are currently "+std::to_string((*channels).size())+" channels.\n";
        send(fd, message.c_str(), strlen(message.c_str()), 0);
        for(itr = (*channels).begin(); itr!=(*channels).end(); itr++) {
            message = "* " +itr->first.substr(1)+"\n";
            send(fd, message.c_str(), strlen(message.c_str()), 0);
        }
        return;
    }

	std::vector<int> userlist = *(itr->second);
	
    std::string message = "There are currently "+std::to_string(userlist.size())+" members.\n"+comm+" members: ";
	send(fd, message.c_str(), strlen(message.c_str()), 0);

	std::unordered_map<int, std::string>::iterator itr2;
	std::string name;
	for (unsigned int i = 0; i < userlist.size(); i++) {
		int namefd = userlist[i];
		itr2 = (*users).find(namefd);
		if (itr2 != (*users).end()) {
			name = itr2->second;
		}
		else {
			perror("List User Not Found");
			return;
		}
		send(fd, (name+" ").c_str(), strlen((name+" ").c_str()), 0);
	}
    send(fd, "\n", 1, 0);



}

void join(int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int>* >* channels
			, std::unordered_map<int, std::string>* users
			) {
    printf("Joining to channel: %s\n", comm.c_str());
	if ((*channels).find(comm) == (*channels).end())
	{
        printf("Didn't find channel.\n");
		std::vector<int> *v = new std::vector<int>();
		v->push_back(fd);
		//std::pair<std::string, std::vector<int> > channelPair(comm, v);
		(*channels)[comm] = v;
	}
	else {
        printf("Found channel.\n");
		(*channels)[comm]->push_back(fd);
        std::string message = (*users)[fd]+" joined the channel.\n";
        sendToChannel(message, *(*channels)[comm]);
	}
	
}

bool removeFromChannel(int fd
                        , std::string ch
                        , std::unordered_map<std::string, std::vector<int>* >* channels
                        , std::unordered_map<int, std::string>* users
                        ) {
    std::unordered_map<std::string, std::vector<int>* >::iterator it = (*channels).find(ch);

    if(it == (*channels).end())
        return false;

    std::string name = (*users)[fd];
    std::vector<int>::iterator it2 = (*(it->second)).begin();
    while(it2!=(*(it->second)).end()) {
        if(*it2==fd) {
            *(it->second)->erase(it2);
            std::string message = name+" left the channel.";
            sendToChannel(message, *(it->second));
            return true;
        }
    }
    return false;
    
}

void part(int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int>* >* channels
			, std::unordered_map<int, std::string>* users
			) {

    if(comm.size()==0) {
        for(std::pair<std::string, std::vector<int> *> ch : (*channels)) {
            removeFromChannel(fd, ch.first, channels, users);
        }
    }

	std::unordered_map<std::string, std::vector<int>* >::iterator itr = (*channels).find(comm);
	if (itr != (*channels).end()) {
        if(!removeFromChannel(fd, itr->first, channels, users)) {
            //Print channel does not exist
            std::string message = "You are not currently in "+comm+".\n";
            send(fd, message.c_str(), strlen(message.c_str()), 0);
        }
	}
}

void op(int fd
		, std::string pass
		, std::vector<int>* operators
        , std::string actual_pass
        , bool haspass
		) {
    if(haspass) {
        if(pass!=actual_pass) {
            std::string message = "Invalid OPERATOR command.\n";
            send(fd, message.c_str(), strlen(message.c_str()), 0);
            return;
        }
    }
    (*operators).push_back(fd);
    std::string message = "OPERATOR status bestowed.\n";
    send(fd, message.c_str(), strlen(message.c_str()), 0);

}

void kick(	int fd
			, std::string comm
			, std::unordered_map<std::string, std::vector<int>* >* channels
			, std::unordered_map<int, std::string>* users
			, std::vector<int>* operators
			) {

	int space = comm.find_first_of(' ');
    std::string channel = comm.substr(0, space);
    if(channel[0]!='#') {
        return;
    }
    bool isOp = false;
    for(unsigned int i=0; i<operators->size(); i++) {
        if((*operators)[i]==fd) {
            isOp=true;
            break;
        }
    }
    if(!isOp) {
        return;
    }
	std::string name = comm.substr(space+1);
	int namefd;
	std::string message;
	
	std::unordered_map<int, std::string>::iterator itr;
	for (itr = (*users).begin(); itr != (*users).end(); itr++) {
		if (itr->second == name) {
			namefd = itr->first;
			break;
		}
	}

	if (itr == (*users).end()) {
		message = "This user does not exist.\n";
		send(fd, message.c_str(), strlen(message.c_str()), 0);
		return;
	}

	std::unordered_map<std::string, std::vector<int>* >::iterator channel_itr = (*channels).find(channel);

	if (channel_itr != (*channels).end()) {
		std::vector<int> v = *(channel_itr->second);
		std::vector<int>::iterator itr2;
		for (itr2 = v.begin(); itr2 != v.end(); itr2++) {
			if (namefd == *itr2) {
				std::string message = "An operator has kicked you from "+channel+".\n";
				send(*itr2, message.c_str(), strlen(message.c_str()), 0);
				v.erase(itr2);
				message = name+" was successfully removed from "+channel+".\n";
				send(fd, message.c_str(), strlen(message.c_str()), 0);
				return;
			}
		}
		message = name+" is not in channel "+channel+"!\n";
		send(fd, message.c_str(), strlen(message.c_str()), 0);
	}

	else {
		message = channel+" does not exist!\n";
		send(fd, message.c_str(), strlen(message.c_str()), 0);
	}
		
	
}

void privmsg(	int fd
				, std::string comm
				, std::unordered_map<std::string, std::vector<int>* >* channels
				, std::unordered_map<int, std::string>* users
				) {

	int space = comm.find_first_of(' ');
    std::string receiver = comm.substr(0, space);
	std::string message = comm.substr(space+1);
	std::unordered_map<std::string, std::vector<int>* >::iterator itr = (*channels).find(receiver);
	if (itr != (*channels).end()) {
		sendToChannel(message, *(itr->second));
		return;
	}

	std::unordered_map<int, std::string>::iterator itr2;
	for (itr2 = (*users).begin(); itr2 != (*users).end(); itr2++) {
		if (itr2->second == receiver) {
			send(itr2->first, message.c_str(), strlen(message.c_str()), 0);
			return;
		}
	}
	
	message = "No user or channel of that name.\n";
	send(fd, message.c_str(), strlen(message.c_str()), 0);
	return;
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
		printf("%s\n", pass.c_str());
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
    std::unordered_map<std::string, std::vector<int>* > channels;
    std::vector<int> operators;

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    int nready = 0;
    int readlen = 0;

    while(1) {

        printf("\n");
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
    		if(nready<=0) {
                printf("Done after listenfd.\n");
    			continue;
            }
    	}

    	std::vector<int>::iterator i;

    	for(i = waitingfd.begin(); i<waitingfd.end(); i++) {
    		int fd = *i;
    		if(FD_ISSET(fd, &rset)) {

    			memset(buffer, 0, 512);
    			readlen = read(fd, buffer, 512);

                std::string temp(buffer, readlen-1);
                std::cout<<"Handling new user that sent command:\n"+temp+"\n";
    			if(readlen==0) {/*USER HAS DISCONNECTED*/
    				FD_CLR(fd, &allset);
    				close(fd);
    			}
    			else {/*PROCESS COMMAND*/
    				
    				unsigned int space = temp.find_first_of(' ');
    				std::string comm = temp.substr(0, space);
    				if(space==std::string::npos) {/*NO SPACES FOUND*/
    					invalidCommand(fd);
    					FD_CLR(fd, &allset);
    					close(fd);
    				}
    				else if(comm=="USER") {/*VALID COMMAND*/
    					std::string name = temp.substr(space+1);
    					addUser(fd, name, &users);/****************************************************/
    					
    				}
    				else {/*INVALID COMMAND*/
    					memset(buffer, 0, 512);
    					strcpy(buffer, "Invalid command, please identify yourself with USER.\n");
    					send(fd, buffer, strlen(buffer), 0);
    					FD_CLR(fd, &allset);
    					close(fd);
    				}
    			}
    			nready--;
                i = waitingfd.erase(i);/*DELETE FD FROM WAITING VECTOR*/
                i--;
    			if(nready<=0) {
                    printf("Done after waitingfd.\n");
    				continue;
                }
    			
    		}
    	}

    	for(std::pair<int, std::string> u : users) {
    		int fd = u.first;
    		if(FD_ISSET(fd, &rset)) {

    			memset(buffer, 0, 512);
    			readlen = read(fd, buffer, 512);

                std::string buff(buffer, readlen-1);
                std::cout<<"Responding to command:\n"+buff+"\n";

    			if(readlen==0) {/*CLIENT HAS DISCONNECTED*/
    				quit(fd, &channels, &users, &operators);/**************************************************************/
    			     FD_CLR(fd, &allset);
                     close(fd);
                }
    			else {
    				unsigned int space = buff.find_first_of(' ');

    				std::string temp = buff.substr(0, space);
    				if(temp=="LIST") {/*LIST COMMAND*/
                        std::cout<<"LIST\n";
    					list(fd, buff.substr(space+1), &channels, &users);/*************************************/
    				}
    				else if(temp=="JOIN") {/*JOIN COMMAND*/
                        std::cout<<"JOIN\n";
    					join(fd, buff.substr(space+1), &channels, &users);/*************************************/
    				}
    				else if(temp=="PART") {/*PART COMMAND*/
                        std::cout<<"PART\n";
    					part(fd, buff.substr(space+1), &channels, &users);/*************************************/
    				}
    				else if(temp=="OPERATOR") {/*OPERATOR COMMAND*/
                        std::cout<<"OPERATOR\n";
    					op(fd, buff.substr(space+1), &operators, pass, haspass);/***************************************/
    				}
    				else if(temp=="KICK") {/*KICK COMMAND*/
                        std::cout<<"KICK\n";
    					kick(fd, buff.substr(space+1), &channels, &users, &operators);/*************************************/
    				}
    				else if(temp=="PRIVMSG") {/*PRIVMSG COMMAND*/
                        std::cout<<"PRIVMSG\n";
    					privmsg(fd, buff.substr(space+1), &channels, &users);/**********************************/
    				}
    				else if(temp=="QUIT") {/*QUIT COMMAND*/
                        std::cout<<"QUIT\n";
    					quit(fd, &channels, &users, &operators);/***********************************************************/
    				    FD_CLR(fd, &allset);
                        close(fd);
                    }
    				else {/*INVALID ERROR*/
    					invalidCommand(fd);
    				}
    			}

    			nready--;
    			if(nready<=0) {
                    printf("Done after reg.\n");
    				continue;
                }
    		}
    	}
    }


	return 0;
}