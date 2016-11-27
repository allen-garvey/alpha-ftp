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
   * validate command line arguments 
   */
  if(argc != 2){
    printUsage(argv[0]);
    exit(1);
  }
  //get port number from command line arguments
  int portNum = atoi(argv[1]);
  //check that portNum is valid - if atoi fails, 0 is returned
  if(portNum <= 0 || portNum > 65535){
    printUsage(argv[0]);
    exit(1);
  }

  /* 
   * socket: create the parent socket 
   */
  int parentSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (parentSocketFileDescriptor < 0){
    error("ERROR opening TCP socket");
  }

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  int optval = 1; //doesn't really do anything, but required for setsockopt
  setsockopt(parentSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  struct sockaddr_in serverAddress; /* server's addr */
  bzero((char *) &serverAddress, sizeof(serverAddress));

  //set internet address
  serverAddress.sin_family = AF_INET;
  //use system's ip address
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  //set listen port
  serverAddress.sin_port = htons((unsigned short)portNum);

  /* 
   * bind: associate the parent socket with a port 
   */
  if(bind(parentSocketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "Could not bind to port %d\n", portNum);
    exit(1);
  }

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentSocketFileDescriptor, 5) < 0){ /* allow 5 requests to queue up */ 
    error("ERROR on listen");
  }

  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  struct sockaddr_in clientAddress; /* client addr */
  uint clientAddressLength = sizeof(clientAddress);
  //set aside memory for messages to/from client
  char messageBuffer[BUFFER_SIZE];
  while (1) {
    /* 
     * accept: wait for a connection request 
     */
    int clientFileDescriptor = accept(parentSocketFileDescriptor, (struct sockaddr *) &clientAddress, &clientAddressLength);
    if(clientFileDescriptor < 0){
      error("ERROR while trying to accept connection");
    }
    
    /* 
     * read: read input string from the client
     */
    bzero(messageBuffer, BUFFER_SIZE);
    int charCountTransferred = read(clientFileDescriptor, messageBuffer, BUFFER_SIZE);
    if(charCountTransferred < 0){
      error("ERROR reading from socket");
    }
    printf("server received %d bytes: %s", charCountTransferred, messageBuffer);
    
    /* 
     * write: echo the input string back to the client 
     */
    charCountTransferred = write(clientFileDescriptor, messageBuffer, strlen(messageBuffer));
    if(charCountTransferred < 0){
      error("ERROR writing to socket");
    }
    //close connection
    close(clientFileDescriptor);
  }
}