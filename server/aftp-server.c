/* 
 * Alpha-ftp
 * by: Allen Garvey
 * usage: aftp-server <port>
 * based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//used for the buffer for messages sent to/from the client
#define BUFFER_SIZE 1024


/*
 * Error functions
 */
//prints program usage
void printUsage(char *programName){
  fprintf(stderr, "usage: %s <port>\n", programName);
}

void error(char *msg){
  perror(msg);
  exit(1);
}

int main(int argc, char **argv){
  /* 
   * check command line arguments 
   */
  if(argc != 2){
    printUsage(argv[0]);
    exit(1);
  }
  int portNum = atoi(argv[1]);
  //check that portNum is valid - if atoi fails, 0 is returned
  if(portNum <= 0 || portNum > 65535){
    printUsage(argv[0]);
    exit(1);
  }

  /* 
   * socket: create the parent socket 
   */
  int parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0){
    error("ERROR opening socket");
  }

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  int optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  struct sockaddr_in serveraddr; /* server's addr */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portNum);

  /* 
   * bind: associate the parent socket with a port 
   */
  if(bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
    error("ERROR on binding");
  }

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0){ /* allow 5 requests to queue up */ 
    error("ERROR on listen");
  }

  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  struct sockaddr_in clientaddr; /* client addr */
  uint clientlen = sizeof(clientaddr);
  char buf[BUFFER_SIZE]; /* message buffer */
  while (1) {
    /* 
     * accept: wait for a connection request 
     */
    int childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0){
      error("ERROR on accept");
    }
    
    /* 
     * gethostbyaddr: determine who sent the message 
     */
    struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL){
      error("ERROR on gethostbyaddr");
    }
    char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if(hostaddrp == NULL){
      error("ERROR on inet_ntoa\n");
    }
    
    /* 
     * read: read input string from the client
     */
    bzero(buf, BUFFER_SIZE);
    int n = read(childfd, buf, BUFFER_SIZE);
    if (n < 0){
      error("ERROR reading from socket");
    }
    printf("server received %d bytes: %s", n, buf);
    
    /* 
     * write: echo the input string back to the client 
     */
    n = write(childfd, buf, strlen(buf));
    if (n < 0){
      error("ERROR writing to socket");
    }

    close(childfd);
  }
}