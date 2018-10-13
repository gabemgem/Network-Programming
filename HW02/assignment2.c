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

struct namestruct{
  char n[100];
};

int main(int argc, char ** argv){
  int sockfd, client_set[5], i, sd, master, new_socket, value;
  struct sockaddr_in cliaddr, servaddr;
  char secret_word[1024], buffer[1024];
  struct namestruct names[5];
  fd_set rset;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  for (i = 0; i < 5; i++) {
    client_set[i] = 0;
    struct namestruct name;
    strcpy(name.n, "");
    names[i] = name;
  }

  while(1){
    FD_ZERO(&rset);
    FD_SET(0, &rset);
    for (i = 0; i < 5; i++){
      sd = client_set[i];
      if(sd > 0) {
        FD_SET(sd, &rset);
      }
    }

    printf("Waiting for selection\n");
    select(FD_SETSIZE, &rset, NULL, NULL, NULL);
    printf("Select finished\n\n");

    //If new incoming connection
    if (FD_ISSET(master, &rset)){
      if ((new_socket = accept(master, (struct sockaddr*)&cliaddr, (socklen_t*)sizeof(&cliaddr)))<0){
        perror("Accept");
        exit(EXIT_FAILURE);
      }
      printf("New Connection Has been Made");
      char* user_message = "Welcome! Please Enter a Username";
      for (i = 0; i < 5; i++){
        if(client_set[i] == 0){
          client_set[i] = new_socket;
          printf("Added to list of clients #%d\n", i);
          break;
        }
      }
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
          //Check if username is null -> check if name is not taken
          //or process input as a guessed word

        }
      }
    }
  }
}
