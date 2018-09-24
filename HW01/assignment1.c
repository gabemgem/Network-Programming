//#include "unpv13e/lib/unp.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <resolv.h>
#include <strings.h>
#include <netinet/in.h>
#include "packet_handler.h"
#include "file_handler.h"
//#include "unp.h"

#define BUFFSIZE 516
#define sendrecvflag 0
transaction t;

/*int file_buffer(transaction_t *transaction) {
  if ((lseek(transaction->filedata, transaction->filepos, SEEK_SET)) == -1){
    printf(stderr, "%s\n", strerror(errno));
    return -1;
  }

  if (int bytes_read = read(transaction->filedata, &transaction->filebuffer, 512) == -1) {
    printf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return bytes_read;
}*/


int main(int argc, char **argv) {
  int sockfd, connfd;
  pid_t cpid;
  struct sockaddr_in cliaddr, servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = 0;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  printf("Waiting for connections");
  //Listen(sockfd, LISTENQ);
  while(1){
    //connfd = Accept(listenfd, (SA*) &cliaddr, sizeof(cliaddr));
    if (!fork){
      //Area to handle client
      char buf[BUFFSIZE];
      char databuff[0];
      int numbytes;
      numbytes = recvfrom(sockfd, buf, BUFFSIZE, sendrecvflag, (struct sockaddr*) &cliaddr, sizeof(cliaddr));
      printf("%d, %s\n", numbytes, buf);

      //Process Packet for packet obj
      //file_open_read(t.filename, t.filedata);


    }
  }
}


//Request to read or write file, request connection
//If granted access, connection is opened
//File sent in fixed length blocks of 512 bytes
//Data packet of less than 512 bytes signifies transfer opened

//TFTP header consists of a 2 byte opcode field which indicates packet type
