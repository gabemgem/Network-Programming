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
    int id;
};

int dist(int a, int b) {
    return a^b;
}

int buck(int dist) {
    if(dist==0)
        return 0;
    if(dist>=1 && dist<2)
        return 1;
    if(dist>=2 && dist<4)
        return 2;
    if(dist>=4 && dist<8)
        return 3;
    if(dist>=8 && dist<16)
        return 4;
    if(dist>=16 && dist<32)
        return 5;
    if(dist>=32 && dist<64)
        return 6;
    if(dist>=64 && dist<128)
        return 7;
    if(dist>=128 && dist<256)
        return 8;
    return -1;
}

void sendToTruple(std::string mess, int connfd, threeTuple n) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    bzero(&addr, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(n.name.c_str());
    addr.sin_port = htons(n.port);

    sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&addr, addrlen);
}

threeTuple* closestInBucket(int theirID, std::list<threeTuple>* bucket) {
    std::list<threeTuple>::iterator i = (*bucket).begin();
    int minDist = dist((*i).id, theirID);
    threeTuple* closest = &(*i);
    i++;
    for(;i!=(*bucket).end();i++) {
        int newDist = dist((*i).id, theirID);
        if(newDist<minDist) {
            minDist = newDist;
            closest = &(*i);
        }
    }
    return closest;
}

threeTuple* closestInBucketWithRemove(int theirID, std::list<threeTuple>* bucket) {
    std::list<threeTuple>::iterator i = (*bucket).begin();
    std::list<threeTuple>::iterator minIt = i;
    int minDist = dist((*i).id, theirID);
    threeTuple* closest = &(*i);
    i++;
    for(;i!=(*bucket).end();i++) {
        int newDist = dist((*i).id, theirID);
        if(newDist<minDist) {
            minDist = newDist;
            closest = &(*i);
            minIt = i;
        }
    }
    (*bucket).erase(minIt);
    return closest;
}

int closestFilledBucket(int b, std::vector<std::list<threeTuple> >*table) {
    if((*table)[b].size()>0)
        return b;
    int i = b, j = b;
    while(i!=-1 && j!=9) {
        if((*table)[i].size()>0)
            return i;
        if((*table)[j].size()>0)
            return j;
        if(i>-1)
            i--;
        if(j<9)
            j++;
    }
    return -1;
}

void removeFromBucket(int theirID, std::list<threeTuple>* b) {
    std::list<threeTuple>::iterator i;
    for(i = (*b).begin(); i!=(*b).end(); i++) {
        if((*i).id==theirID) {
            (*b).erase(i);
            return;
        }
    }
}

void addToTable(threeTuple n, std::vector<std::list<threeTuple> >*table, int myID, int k) {
    int b = buck(dist(n.id, myID));
    if((*table)[b].size()>=(unsigned int)k) {
        (*table)[b].pop_front();
    }
    (*table)[b].push_back(n);
}

void connect(std::string comm, std::vector<std::list<threeTuple> >*table, std::string myName, int myID, int connfd, int k) {
    int space = comm.find_first_of(' ');
    std::string theirName = comm.substr(0, space);
    int theirPort = atoi(comm.substr(space+1).c_str());

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(theirName.c_str());
    addr.sin_port = htons(theirPort);

    std::string mess = "HELLO "+myName+" "+std::to_string(myID);
    sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&addr, addrlen);
    
    bzero(buffer, 512);
    int len = recvfrom(connfd, buffer, 512, 0, (struct sockaddr*)&addr, &addrlen);
    printf("%s\n", buffer);
    std::string temp(buffer, len);
    
    space = temp.find_first_of(' ');
    std::string comm2 = temp.substr(0, space);
    if(comm2!="MYID")
        return;
    
    int theirID = atoi(temp.substr(space+1).c_str());
    int d = dist(theirID, myID);
    if(d>8)
        return;
    
    threeTuple newFriend;
    newFriend.name = theirName;
    newFriend.port = theirPort;
    newFriend.id = theirID;
    if((*table)[buck(d)].size()>=(unsigned int)k)
        (*table)[buck(d)].pop_front();
    (*table)[buck(d)].push_back(newFriend);

}

void find_node(std::string comm, std::vector<std::list<threeTuple> >*table, int connfd, int myID, int k) {
    int theirID = atoi(comm.c_str());
    int asked=0, received=0;
    int closestBucket = buck(dist(theirID, myID));
    std::string mess = "FIND_NODE "+comm;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(connfd, &rset);

    while(asked<k) {
        int b = closestFilledBucket(closestBucket, table);
        if(b==-1)
            return;
        threeTuple* cl = closestInBucket(theirID, &((*table)[b]));
        sendToTruple(mess, connfd, *cl);
        received = 0;
        while(received<k) {
            select(FD_SETSIZE, &rset, NULL, NULL, &tv);
            if(tv.tv_sec==0 && tv.tv_usec==0) {
                removeFromBucket((*cl).id, &((*table)[b]));
            }
            if(FD_ISSET(connfd, &rset)) {
                bzero(buffer, 512);
                int l = recvfrom(connfd, buffer, 512, 0, NULL, NULL);
                std::string temp(buffer, l);
                int space = temp.find_first_of(' ');
                std::string comm = temp.substr(0,space);
                if(comm=="NODE") {
                    temp = temp.substr(space+1);
                    space = temp.find_first_of(' ');
                    std::string sender_name = temp.substr(0,space);
                    temp = temp.substr(space+1);
                    space = temp.find_first_of(' ');
                    int sender_port = atoi(temp.substr(0,space).c_str());
                    if(sender_port==0) {
                        break;
                    }
                    int sender_id = atoi(temp.substr(space+1).c_str());
                    threeTuple newFriend;
                    newFriend.name = sender_name;
                    newFriend.port = sender_port;
                    newFriend.id = sender_id;
                    addToTable(newFriend, table, myID, k);
                    if(sender_id==theirID)
                        return;
                    received++;
                    tv.tv_sec=3;
                    tv.tv_usec=0;
                }
            }
        }
        
        asked++;
    }
}


void find_data(std::string comm) {

}

void value(std::string comm) {

}

void store(std::string comm, std::vector<std::list<threeTuple> >*table, int connfd, int myID) {
    int space = comm.find_first_of(' ');
    int key = atoi(comm.substr(0, space).c_str());
    int value = atoi(comm.substr(space+1).c_str());
    std::string mess = "STORE"+std::to_string(key)+" "+std::to_string(value);
    int d = dist(key, myID);
    int size = (*table)[buck(d)].size()-1;
    if(size<0) {
        while(d<9) {
            d++;
            size = (*table)[buck(d)].size()-1;
            if(size>=0)
                break;
        }
    }

    std::list<threeTuple>::reverse_iterator it = (*table)[buck(d)].rbegin();
    std::list<threeTuple>::reverse_iterator it2 = (*table)[buck(d)].rbegin();
    int minDist = dist(key, (*it).id);
    for(; it2!=(*table)[buck(d)].rend(); it2++) {
        int tempD = dist(key, (*it2).id);
        if(tempD<minDist) {
            minDist = tempD;
            it = it2;
        }
    }

    sendToTruple(mess, connfd, *it);


}



void quit(std::vector<std::list<threeTuple> >*table, int connfd, int myID) {
    std::string mess = "QUIT "+std::to_string(myID);

    for(int i=0; i<9; i++) {
        for(threeTuple n : (*table)[i]) {
            sendToTruple(mess, connfd, n);
        }
    }
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
    int k = atoi(argv[4]);

    std::vector<std::list<threeTuple> > table(9); 
    std::vector<std::pair<int, int> > values;

	bzero(&servaddr, addrlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(port);
	
	if(bind(connfd, (struct sockaddr*)&servaddr, addrlen) == -1) {
		perror("Error binding");
		exit(1);
	}

    std::string myid = "MYID "+std::to_string(id);

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
            len = recvfrom(connfd, buffer, 512, 0, 
                         (struct sockaddr*)&cliaddr, &addrlen); 
            printf("%s\n", buffer);
            std::string temp(buffer, len);
            int space = temp.find_first_of(' ');
            std::string comm = temp.substr(0, space);

            //Send store client data, send MYID message back
            if (comm == "HELLO") {
                temp = temp.substr(space+1);
                space = temp.find_first_of(' ');
                std::string sender_name = temp.substr(0,space);
                std::string sender_id = temp.substr(space+1);
                int sender_port = ntohs(cliaddr.sin_port);
                threeTuple newFriend;
                newFriend.name = sender_name;
                newFriend.port = sender_port;
                newFriend.id = atoi(sender_id.c_str());
                addToTable(newFriend, &table, id, k);
                sendto(connfd, myid.c_str(), myid.size(), 0, (struct sockaddr*)&cliaddr, addrlen);
            }

            else if (comm == "QUIT") {
                int theirID = atoi(temp.substr(space+1).c_str());
                int d = dist(theirID, id);
                std::list<threeTuple>::iterator i;
                for(i = table[d].begin(); i!=table[d].end(); ++i) {
                    if((*i).id==theirID) {
                        table[d].erase(i);
                    }
                }

            }

            else if (comm == "STORE") {
                temp = temp.substr(space+1);
                space = temp.find_first_of(' ');
                int key = atoi(temp.substr(0, space).c_str());
                int value = atoi(temp.substr(space+1).c_str());
                std::pair<int, int> newStorage(key,value);
                values.push_back(newStorage);
                printf("Stored %d with key: %d\n", key, value);
            }

            else if (comm == "FIND_NODE") {
                struct sockaddr_in tempAddr;
                bzero(&tempAddr, addrlen);
                int theirID = atoi(temp.substr(space+1).c_str());
                int b = buck(dist(theirID, id));
                int sent = 0;
                int movement = 0;
                while(sent<k) {
                    if(table[b].size()>0) {
                        std::list<threeTuple> bucket(table[b]);
                        while(sent<k && bucket.size()>0) {
                            threeTuple* n = closestInBucketWithRemove(theirID, &bucket);
                            std::string mess = "NODE "+n->name+" "+std::to_string(n->port)+" "+std::to_string(n->id);
                            sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&cliaddr, addrlen);
                            sent++;
                        }
                    }
                    if(sent<k) {
                        movement = (movement>=0) ? movement+1 : movement-1;
                        movement *= -1;
                        b+=movement;
                        if(b<0 || b>8) {
                            movement = (movement>=0) ? movement+1 : movement-1;
                            movement *= -1;
                            b+=movement;
                            if(b<0 || b>8) {
                                std::string mess = "NODE 0 0 0";
                                sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&cliaddr, addrlen);
                                sent=k;
                            }
                        }
                    }
                    
                }
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
    					connect(line.substr(space+1), &table, name, id, connfd, k);
    				}
            if (comm =="FIND_NODE") {/*LIST COMMAND*/
                        std::cout<<"FIND_NODE\n";
    					find_node(line.substr(space+1), &table, connfd, id, k);
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
    					store(line.substr(space+1), &table, connfd, id);
    				} 
            if (comm =="QUIT") {/*LIST COMMAND*/
                        std::cout<<"QUIT\n";
    					quit(&table, connfd, id);
                        close(connfd);
                        return 0;
    				}             


        }

    }
}



