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
//#include <openssl/sha.h>
#include <openssl/evp.h>

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

threeTuple findThreeTuple(std::string name, std::vector<std::list<threeTuple> >*table) {
    for(std::list<threeTuple> bucket : *table) {
        for(std::list<threeTuple>::iterator i=bucket.begin(); i!=bucket.end(); i++) {
            if(i->name==name) {
                threeTuple tt = *i;
                bucket.erase(i);
                bucket.push_back(tt);
                return tt;
            }
        }
    }
    threeTuple notFound;
    notFound.id=0;
    notFound.name="";
    notFound.port=0;
    return notFound;
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

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(connfd, &rset);

    select(FD_SETSIZE, &rset, NULL, NULL, &tv);

    if(tv.tv_usec==0 && tv.tv_sec==0)
        return;

    int len = recvfrom(connfd, buffer, 512, 0, (struct sockaddr*)&addr, &addrlen);
    
    std::string temp(buffer, len);
    
    space = temp.find_first_of(' ');
    std::string comm2 = temp.substr(0, space);
    if(comm2!="MYID")
        return;
    
    int theirID = atoi(temp.substr(space+1).c_str());
    printf("<%x %s\n", theirID, buffer);
    int b = buck(dist(theirID, myID));
    if(b>8)
        return;
    
    threeTuple newFriend;
    newFriend.name = theirName;
    newFriend.port = theirPort;
    newFriend.id = theirID;
    addToTable(newFriend, table, myID, k);

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
                printf("!%x %s\n", cl->id, mess.c_str());
                removeFromBucket((*cl).id, &((*table)[b]));
                tv.tv_sec=3;
                tv.tv_usec=0;
                continue;
            }
            if(FD_ISSET(connfd, &rset)) {
                printf(">%x %s\n", cl->id, mess.c_str());
                bzero(buffer, 512);
                int l = recvfrom(connfd, buffer, 512, 0, NULL, NULL);

                std::string temp(buffer, l);
                int space = temp.find_first_of(' ');
                std::string comm = temp.substr(0,space);
                if(comm=="NODE") {
                    printf("<%x %s\n", cl->id, buffer);
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


void find_data(std::string comm, std::vector<std::list<threeTuple> >*table, int connfd, int myID, int k) {
    int key = atoi(comm.c_str());
    int asked=0, received=0;
    int closestBucket = buck(dist(key, myID));
    std::string mess = "FIND_DATA "+comm;
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
        threeTuple* cl = closestInBucket(key, &((*table)[b]));
        sendToTruple(mess, connfd, *cl);
        received = 0;
        while(received<k) {
            select(FD_SETSIZE, &rset, NULL, NULL, &tv);
            if(tv.tv_sec==0 && tv.tv_usec==0) {
                printf("!%x %s\n", cl->id, mess.c_str());
                removeFromBucket((*cl).id, &((*table)[b]));
                tv.tv_sec=3;
                tv.tv_usec=0;
                continue;
            }
            if(FD_ISSET(connfd, &rset)) {
                printf(">%x %s\n", cl->id, mess.c_str());
                bzero(buffer, 512);
                int l = recvfrom(connfd, buffer, 512, 0, NULL, NULL);
                std::string temp(buffer, l);
                int space = temp.find_first_of(' ');
                std::string comm = temp.substr(0,space);
                if(comm=="NODE") {
                    printf("<%x %s\n", cl->id, mess.c_str());
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
                    received++;
                    tv.tv_sec=3;
                    tv.tv_usec=0;
                }
                else if(comm=="VALUE") {
                    temp = temp.substr(space+1);
                    space = temp.find_first_of(' ');
                    int theirID = atoi(temp.substr(0, space).c_str());
                    temp = temp.substr(space+1);
                    space = temp.find_first_of(' ');
                    int theirKey = atoi(temp.substr(0, space).c_str());
                    theirKey--;//unused variable - this gets rid of compiler warning
                    int theirValue = atoi(temp.substr(space+1).c_str());
                    printf("Received %d from  %d\n", theirValue, theirID);
                    return;
                }
            }
        }
        
        asked++;
    }

}

void store(std::string comm, std::vector<std::list<threeTuple> >*table, int connfd, int myID) {
    int space = comm.find_first_of(' ');
    int key = atoi(comm.substr(0, space).c_str());
    int value = atoi(comm.substr(space+1).c_str());
    std::string mess = "STORE"+std::to_string(key)+" "+std::to_string(value);

    int closestBucket = buck(dist(key,myID));
    int b = closestFilledBucket(closestBucket, table);
    if(b==-1)
        return;

    threeTuple* cl = closestInBucket(key, &((*table)[b]));
    sendToTruple(mess, connfd, *cl); 
    printf(">%x %s\n", cl->id, mess.c_str());
}



void quit(std::vector<std::list<threeTuple> >*table, int connfd, int myID) {
    std::string mess = "QUIT "+std::to_string(myID);

    for(int i=0; i<9; i++) {
        for(threeTuple n : (*table)[i]) {
            sendToTruple(mess, connfd, n);
            printf(">%x %s\n", n.id, mess.c_str());
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
    char* seed = argv[3];
    int k = atoi(argv[4]);

    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char idhash[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(mdctx, seed, strlen(seed));
    EVP_DigestFinal_ex(mdctx, idhash, &md_len);
    EVP_MD_CTX_destroy(mdctx);

    int id = (int) idhash[0];
    printf("%d -> %x\n\n", id, id);


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

    std::string myid = "MYID "+ std::to_string(id);

    FD_ZERO(&allset);
    FD_ZERO(&rset);
    FD_SET(connfd, &allset);
    FD_SET(0, &allset);
    int nready=0;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    bool connectAfter=false;

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
            len = recvfrom(connfd, buffer, 512, 0, 
                         (struct sockaddr*)&cliaddr, &addrlen); 
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cliaddr.sin_addr, str, INET_ADDRSTRLEN);
            std::string theirName(str);
            int theirPort = ntohs(cliaddr.sin_port);
            threeTuple them = findThreeTuple(theirName, &table);

            std::string temp(buffer, len);
            int space = temp.find_first_of(' ');
            std::string comm = temp.substr(0, space);

            if(them.port==0) {
                printf("<? %s\n", buffer);
                if(comm!="HELLO") {
                    connectAfter=true;
                }
                
            }
            else {
                printf("<%x %s\n", them.id, buffer);
            }

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
                printf(">%x %s\n", newFriend.id, myid.c_str());
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
                            printf(">%x %s\n", theirID, mess.c_str());
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
                                printf(">%x, %s\n", theirID, mess.c_str());
                                sent=k;
                            }
                        }
                    }
                    
                }
            }

            else if (comm == "FIND_DATA") {
                struct sockaddr_in tempAddr;
                bzero(&tempAddr, addrlen);
                int theirKey = atoi(temp.substr(space+1).c_str());
                bool hasKey = false;
                for(std::pair<int,int> kv : values) {
                    if(kv.first == theirKey) {
                        std::string mess = "VALUE"+std::to_string(id)+" "+std::to_string(kv.first)+" "+std::to_string(kv.second);
                        sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&cliaddr, addrlen);
                        if(them.port==0) {
                            printf(">? %s\n", mess.c_str());
                        }
                        else {
                            printf(">%x %s\n", them.id, mess.c_str());
                        }
                        hasKey=true;
                        break;
                    }
                }
                if(!hasKey) {
                    int b = buck(dist(theirKey, id));
                    int sent = 0;
                    int movement = 0;
                    while(sent<k) {
                        if(table[b].size()>0) {
                            std::list<threeTuple> bucket(table[b]);
                            while(sent<k && bucket.size()>0) {
                                threeTuple* n = closestInBucketWithRemove(theirKey, &bucket);
                                std::string mess = "NODE "+n->name+" "+std::to_string(n->port)+" "+std::to_string(n->id);
                                sendto(connfd, mess.c_str(), mess.size(), 0, (struct sockaddr*)&cliaddr, addrlen);
                                if(them.port==0) {
                                    printf(">? %s\n", mess.c_str());
                                }
                                else {
                                    printf(">%x %s\n", them.id, mess.c_str());
                                }
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
                                    if(them.port==0) {
                                        printf(">? %s\n", mess.c_str());
                                    }
                                    else {
                                        printf(">%x %s\n", them.id, mess.c_str());
                                    }
                                    sent=k;
                                }
                            }
                        }
                        
                    }
                }
            }

            if(connectAfter) {
                connectAfter=false;
                sleep(2);
                std::string connectComm = theirName+" "+std::to_string(theirPort);
                connect(connectComm, &table, name, id, connfd, k);
            }
            
            nready--;
        }

        else if (nready>0 && FD_ISSET(0, &rset)) {//handle incoming commands
            std::getline(std::cin, line);
            unsigned int space = line.find_first_of(' ');
            std::string comm = line.substr(0, space);  

            if (comm =="CONNECT") {/*LIST COMMAND*/
    					connect(line.substr(space+1), &table, name, id, connfd, k);
    				}
            if (comm =="FIND_NODE") {/*LIST COMMAND*/
    					find_node(line.substr(space+1), &table, connfd, id, k);
    				}
            if (comm =="FIND_DATA") {/*LIST COMMAND*/
    					find_data(line.substr(space+1), &table, connfd, id, k);
    				}    
            if (comm =="STORE") {/*LIST COMMAND*/
    					store(line.substr(space+1), &table, connfd, id);
    				} 
            if (comm =="QUIT") {/*LIST COMMAND*/
    					quit(&table, connfd, id);
                        close(connfd);
                        return 0;
    				}             


        }

    }
}



