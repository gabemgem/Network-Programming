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

int MAX_CLIENTS = 5;
int MAX_LINE = 1024;
int ret[2];

struct namestruct{
  char n[100], hasname;
};

int checkWord(char* guess, char* word, int size, int* correct, int* placed){
    //Assume input and word are the same length
    int i, j; 
    *correct = 0;
    *placed = 0;

    char temp[1024];
    strcpy(temp, word);

    for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
        if (guess[i] == temp[j]){
          (*correct)++;

          if (i == j) {
            (*placed)++;
          }

          temp[j] = '\0';
        }
      }
    }

    return 1;
}

char checkName(char* buffer, struct namestruct* names){
  int i;
  for (i=0; i < MAX_CLIENTS; i++){
    if (names[i].hasname){
      if (strcmp(buffer, names[i].n) == 0){
        return 0;
      }
    }
  }

  return 1;
}

int main(int argc, char ** argv){
  int sockfd, listenfd, connfd, maxfd, maxi, client_set[5], i, sd, value, num_clients, size, correct, placed;
  unsigned int myPort;
  struct sockaddr_in cliaddr, servaddr;
  char secret_word[1024], buffer[1024];
  struct namestruct names[5];
  fd_set rset, allset;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = 0;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  if(bind(listenfd, (struct sockaddr*)&servaddr, (socklen_t)sizeof(servaddr)) == -1){
		perror("Error binding");
		exit(1);
	}


  struct sockaddr_in addr;
  socklen_t addrlen = (socklen_t)sizeof(addr);
  getsockname(sockfd, (struct sockaddr *)&addr, &addrlen);
  myPort = ntohs(addr.sin_port);

  if(listen(listenfd, 5)<0) {
     perror("Error listening");
     exit(1);
  }

  printf("Now Listening on Port %u\n", myPort);

  for (i = 0; i < 5; i++) {
    client_set[i] = 0;
    struct namestruct name;
    name.hasname = 0;
  }

  maxfd = listenfd;
  maxi = -1;
  num_clients = 0;

  FD_ZERO(&allset);

  while(1){
    /*FD_ZERO(&rset);
    FD_SET(0, &rset);
    for (i = 0; i < 5; i++){
      sd = client_set[i];
      if(sd > 0) {
        FD_SET(sd, &rset);
      }
    }*/

    rset = allset;
    sleep(1);
    printf("Waiting for selection\n");
    select(MAX_CLIENTS, &rset, NULL, NULL, NULL);
    printf("Select finished\n\n");

    //If new incoming connection
    if (FD_ISSET(listenfd, &rset) && num_clients < 5){ //add num_client barrier
      
      if ((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, (socklen_t*) sizeof(&cliaddr)))<0){
        perror("Error on accept");
        exit(EXIT_FAILURE);
      }
      printf("New Connection Has been Made");
      for (i = 0; i < 5; i++){
        if(client_set[i] < 0){
          client_set[i] = connfd;
          printf("Added to list of clients #%d\n", i);
          strcpy(buffer, "Please Enter a Username");
          send(connfd , buffer, MAX_LINE , 0 );
          num_clients++;
          break;
        }
      }

      FD_SET(connfd, &allset);
    		if(connfd>maxfd)
    			maxfd = connfd;
    		if(num_clients-1>maxi)
    			maxi = num_clients-1;
    		//if(--nready <=0)
    		//	continue;
    }

    for(i=0; i < 5; i++){
      sd = client_set[i];
      if (FD_ISSET(sd, &rset)){
        if ((value = read(sd, buffer, 1024)) == 0) {
          //CLIENT DISCONNECTED
          close(sd);
          client_set[i] = 0;
        }
        else {
          //PROCESS CLIENT INPUT
            //Check if client already has -> check if name is not taken
          if (!(names[i].hasname)){
            if (checkName(buffer, names)){
              names[i].hasname = 1;
              strcpy(names[i].n, buffer);
            }
            else{
              strcpy(buffer, "Username is already take, please choose a new one");
              send(sd , buffer, MAX_LINE , 0 );
            }
            break;
          }


          else{
            //or process input as a guessed word
            read(sd, buffer, 1024);
            checkWord(buffer, secret_word, size, &correct, &placed);
            sprintf(buffer, "%s has guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed", names[i].n, buffer, correct, placed);
            
          }


        }
      }
    }
  }
}
