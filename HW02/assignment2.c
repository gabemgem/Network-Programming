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
#include "dictionaryhandler.h"

int MAX_CLIENTS = 5;
int MAX_LINE = 1024;
int ret[2];

struct namestruct
{
    char n[100], hasname;
};

void removeNL(char *buffer)
{
    char NL = '\n';
    buffer = strtok(buffer, &NL);
}

int checkWord(const char *guess, const char *word, int size, int *correct, int *placed)
{
    //Assume input and word are the same length
    int i, j;
    *correct = 0;
    *placed = 0;

    char temp[1024];
    strcpy(temp, word);

    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            if(guess[i] == temp[j])
            {
                (*correct)++;
                temp[j] = ' ';
                break;
            }
        }
    }

    for(int k = 0; k < size; k++)
    {
        if(guess[k] == word[k])
        {
            (*placed)++;
        }
    }

    return 0;
}

char checkName(char *buffer, struct namestruct *names)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (names[i].hasname >= 0)
        {
            if (strcmp(buffer, names[i].n) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Argument error: Please pass in dictionary file and max word length.\n");
        return 1;
    }

    int listenfd, connfd, maxfd, maxi, client_set[5], i, sd, value, num_clients, size, correct, placed;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen = sizeof(cliaddr);
    char secret_word[MAX_LINE], buffer[MAX_LINE], guess[MAX_LINE], trash[MAX_LINE];
    struct namestruct names[5];
    fd_set rset, allset;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = 0;
    if(bind(listenfd, (struct sockaddr *)&servaddr, (socklen_t)sizeof(servaddr)) == -1)
    {
        perror("Error binding");
        exit(1);
    }


    struct sockaddr_in addr;
    unsigned int myPort;
    bzero(&addr, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    getsockname(listenfd, (struct sockaddr *) &addr, &addrlen);
    myPort = ntohs(addr.sin_port);
    printf("%u\n", myPort);

    if(listen(listenfd, 5) < 0)
    {
        perror("Error listening");
        exit(1);
    }


    for (i = 0; i < 5; i++)
    {
        client_set[i] = -1;
        names[i].hasname = -1;
        memset(names[i].n, ' ', sizeof(names[i].n));
    }

    num_clients = 0;

    srand(time(NULL));
    if(dictionary_setup(argv[1], atoi(argv[2])) < 0)
    {
        printf("Error setting up dict\n");
        exit(1);
    }

    size = getWord(secret_word);
    printf("Secret word: %s\n", secret_word);


    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    int nready = 0;

    while(1)
    {

        rset = allset;
        printf("Waiting for selection\n");
        nready = select(FD_SETSIZE, &rset, NULL, NULL, NULL);
        printf("Select finished\n\n");

        //If new incoming connection
        if (FD_ISSET(listenfd, &rset))  //add num_client barrier
        {
            read(listenfd, buffer, 1024);
            printf("Read from listen: %s\n", buffer);
            if(num_clients>=5) {
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
                memset(buffer, ' ', MAX_LINE);
                strcpy(buffer, "Sorry we have the max number of players right now.\nPlease try again later.\n");
                send(connfd, buffer, strlen(buffer), 0 );
                close(connfd);
            }
            else {
                if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
                {
                    perror("Error on accept");
                    exit(EXIT_FAILURE);
                }
                printf("New Connection Has been Made\n");
                for (i = 0; i < 5; i++)
                {
                    if(client_set[i] < 0)
                    {
                        client_set[i] = connfd;
                        printf("Added to list of clients #%d\n", i);
                        memset(buffer, ' ', MAX_LINE);
                        strcpy(buffer, "Please Enter a Username\n");
                        send(connfd, buffer, strlen(buffer), 0 );
                        num_clients++;
                        break;
                    }
                }

                FD_SET(connfd, &allset);
                nready--;
                if(nready <= 0)
                    continue;
            }
        }

        for(i = 0; i < 5; i++)
        {
            sd = client_set[i];
            if (FD_ISSET(sd, &rset))
            {
                memset(buffer, 0, MAX_LINE);
                value = read(sd, buffer, 1024);
                if (value == 0)
                {
                    //CLIENT DISCONNECTED
                    printf("%s has disconnected\n", names[i].n);
                    FD_CLR(sd, &allset);
                    close(sd);
                    client_set[i] = -1;
                    names[i].hasname = -1;
                    memset(names[i].n, 0, sizeof(names[i].n));
                    num_clients--;

                }
                else
                {
                    //PROCESS CLIENT INPUT
                    removeNL(buffer);
                    //Check if client already has -> check if name is not taken
                    if (names[i].hasname < 0)
                    {
                        printf("Putting a new username into slot #%d\n", i);

                        if (checkName(buffer, names) == 0)
                        {
                            names[i].hasname = 1;
                            strcpy(names[i].n, buffer);
                            memset(buffer, ' ', MAX_LINE);
                            sprintf(buffer, "Welcome %s, the length of the secret word is %d\nPlease enter a guess:\n", names[i].n, size);
                            printf("Everyone please welcome %s\n", names[i].n);
                        }
                        else
                        {
                            memset(buffer, ' ', MAX_LINE);
                            strcpy(buffer, "Username is already take, please choose a new one.\n");
                            printf("What a dork their username was already taken.\n");
                        }
                        send(sd, buffer, strlen(buffer), 0 );
                        break;
                    }


                    else
                    {
                        //or process input as a guessed word
                        strcpy(guess, buffer);
                        checkWord(guess, secret_word, size, &correct, &placed);
                        memset(buffer, ' ', MAX_LINE);
                        if(placed == size)
                        {
                            sprintf(buffer, "%s has correctly guessed the word %s\nYou are now being disconnected, goodbye.\n", names[i].n, secret_word);
                        }
                        else
                        {
                            printf("%s has guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", names[i].n, guess, correct, placed);
                            sprintf(buffer, "%s has guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", names[i].n, guess, correct, placed);
                        }
                        for(i = 0; i < 5; ++i)
                        {
                            if(client_set[i] != -1)
                            {
                                send(client_set[i], buffer, strlen(buffer), 0);
                            }
                            if(placed == size)
                            {
                                read(listenfd, trash, 1024);
                                close(client_set[i]);
                                client_set[i] = -1;
                                names[i].hasname = -1;
                                memset(names[i].n, 0, sizeof(names[i].n));
                            }
                        }
                        if(placed == size)
                        {
                            size = getWord(secret_word);
                            printf("New secret word: %s\n", secret_word);
                            num_clients=0;
                            FD_ZERO(&allset);
                            FD_SET(listenfd, &allset);

                        }


                    }


                }
            }
        }
    }
    dictionary_close();
    return 0;
}
