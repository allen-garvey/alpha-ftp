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
//number of requests that allowed to queue up waiting for server to become available
#define REQUEST_QUEUE_SIZE 8

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

/*
 * Validate command-line arguments
 */
//Validates command-line arguments for port number to listen on
//returns port number to listen on
int validateCommandLineArguments(int argc, char **argv){
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
  return portNum;
}

/*
 * Setup functions
 */
//builds server address we will use to accept connections
void buildServerAddress(struct sockaddr_in *serverAddress, int portNum){
  //initialize memory by setting it to all 0s
  bzero((char *) serverAddress, sizeof(*serverAddress));
  //set internet address
  serverAddress->sin_family = AF_INET;
  //use system's ip address
  serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
  //set listen port
  serverAddress->sin_port = htons((unsigned short)portNum);
}

//starts the server listening on portNum on IP address determined by operating system
//returns fileDescriptor for the listening socket
int initializeServer(int portNum){
   //create the server socket 
  int serverSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocketFileDescriptor < 0){
    error("ERROR opening TCP socket");
  }

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  int optval = 1; //doesn't really do anything, but required for setsockopt
  setsockopt(serverSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  //build and initialize server address for ip address and port
  struct sockaddr_in serverAddress;
  buildServerAddress(&serverAddress, portNum);

  //bind server to port
  if(bind(serverSocketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "Could not bind to port %d\n", portNum);
    exit(1);
  }

  //start server listening - set queue length to size of REQUEST_QUEUE_SIZE
  if(listen(serverSocketFileDescriptor, REQUEST_QUEUE_SIZE) < 0){
    error("ERROR on listen");
  }
  return serverSocketFileDescriptor;
}


int main(int argc, char **argv){
  //get port number from command-line arguments
  //will print usage and exit if command-line arguments are invalid
  int portNum = validateCommandLineArguments(argc, argv);

  //do server setup and start server listing on portNum
  int serverSocketFileDescriptor = initializeServer(portNum);

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
    int clientFileDescriptor = accept(serverSocketFileDescriptor, (struct sockaddr *) &clientAddress, &clientAddressLength);
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

  //stop server listening
  close(serverSocketFileDescriptor);
}