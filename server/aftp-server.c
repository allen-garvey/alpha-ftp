/* 
 * Alpha-ftp
 * by: Allen Garvey
 * usage: aftp-server <port>
 * server based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
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
//for directory listing
#include <dirent.h>
//for file opening errors
#include <errno.h>

//used for the buffer for messages sent to/from the client
#define MESSAGE_BUFFER_SIZE 1024
//number of requests that allowed to queue up waiting for server to become available
#define REQUEST_QUEUE_SIZE 8
//character sequence to signify end of message sent
#define MESSAGE_END_STRING "\n"

/*
 * Error functions
 */
//prints program usage
void printUsage(char *programName){
  fprintf(stderr, "usage: %s <port>\n", programName);
}

//prints error message and exits program with error code
//based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
void error(char *msg){
  perror(msg);
  exit(1);
}

/*
 * Validate command-line arguments
 */
//validates argument is valid port number
//returns 1 (true) if it is valid, otherwise 0 (false)
int isPortNumValid(int portNum){
  if(portNum > 0 && portNum <= 65535){
    return 1;
  }
  return 0;
}

//Validates command-line arguments for port number to listen on
//returns port number to listen on
//based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
int validateCommandLineArguments(int argc, char **argv){
  if(argc != 2){
    printUsage(argv[0]);
    exit(1);
  }
  //get port number from command line arguments
  int portNum = atoi(argv[1]);
  //check that portNum is valid - if atoi fails, 0 is returned
  if(!isPortNumValid(portNum)){
    printUsage(argv[0]);
    exit(1);
  }
  return portNum;
}

/*
 * Server setup functions
 */
//builds server address we will use to accept connections
//based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
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
//based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
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

/*
* Helper functions for reading/writing to sockets
*/
//sends message to client identified by file descriptor
void sendToSocket(int clientFileDescriptor, char *message){
  //send message to client
  int charCountTransferred = write(clientFileDescriptor, message, strlen(message));
  //check for errors writing
  if(charCountTransferred < 0){
    error("ERROR writing to socket");
  }
}

//reads message from client identified by file descriptor and puts message into 
//buffer supplied as second argument
//based on: http://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c
void readFromSocketIntoBuffer(int clientFileDescriptor, char messageBuffer[MESSAGE_BUFFER_SIZE]){
    //zero the buffer so that old messages do not remain
    bzero(messageBuffer, MESSAGE_BUFFER_SIZE);
    //store message from client in buffer
    int charCountTransferred = read(clientFileDescriptor, messageBuffer, MESSAGE_BUFFER_SIZE);
    //check for errors reading
    if(charCountTransferred < 0){
      error("ERROR reading from socket");
    }
}

/*
* List directory contents functions (-l)
*/
//sends listing of files in current directory to client using socket file descriptor
//requires POSIX compatible OS
//based on: http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
void sendDirectoryListing(int clientFileDescriptor){
  struct dirent *directoryEntry;
  DIR *directory = opendir(".");
  if(directory){
    while((directoryEntry = readdir(directory)) != NULL){
      sendToSocket(clientFileDescriptor, directoryEntry->d_name);
      sendToSocket(clientFileDescriptor, "\n");
    }
    closedir(directory);
  }
  else{
    sendToSocket(clientFileDescriptor, "Directory doesn't exist or can't be accessed");
  }
  //send message end escape sequence so client knows that is the end of the message
  sendToSocket(clientFileDescriptor, MESSAGE_END_STRING);
}

/*
* Send file contents to client functions (-g)
*/
//function to send error message to client when opening file fails
//first argument is file descriptor for client socket connection
//second argument should be reference to errno which will contain error
void sendFileOpenError(int clientFileDescriptor, int errorNum){
  switch(errorNum){
    //permissions error
    case EACCES:
      sendToSocket(clientFileDescriptor, "Permissions denied to open file");
      break;
    //file doesn't exist
    case ENOENT:
      sendToSocket(clientFileDescriptor, "File doesn't exist");
      break;
    //unspecified error
    default:
      break;
  }
}

//sends contents of file identified by fileName argument over socket to client
//identified by clientFileDescriptor
//based on: http://stackoverflow.com/questions/3501338/c-read-file-line-by-line
void sendFileContents(int clientFileDescriptor, char *fileName){
  //attempt to open to specified file for reading
  FILE *filePointer = fopen(fileName, "r");
  //check if it succeed
  if(filePointer == NULL){
    //send error to client, since we couldn't open file
    sendFileOpenError(clientFileDescriptor, errno);
    return;
  }
  //initialize line reading variables
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  //read file line by line and send to client
  while((read = getline(&line, &len, filePointer)) != -1){
    sendToSocket(clientFileDescriptor, line);
  }

  //free space allocated for line
  free(line);
  //close file
  fclose(filePointer);
}


int main(int argc, char **argv){
  //get port number from command-line arguments
  //will print usage and exit if command-line arguments are invalid
  int portNum = validateCommandLineArguments(argc, argv);

  //do server setup and start server listing on portNum
  int serverSocketFileDescriptor = initializeServer(portNum);


  //set aside memory for client connection address
  struct sockaddr_in clientAddress;
  //initialize address length, since it is used to accept connection
  uint clientAddressLength = sizeof(clientAddress);
  //set aside memory for messages to/from client
  char messageBuffer[MESSAGE_BUFFER_SIZE];
  //main server listen loop
  while(1){
    //accept connection
    int clientFileDescriptor = accept(serverSocketFileDescriptor, (struct sockaddr *) &clientAddress, &clientAddressLength);
    if(clientFileDescriptor < 0){
      error("ERROR while trying to accept connection");
    }
    
    //read message sent from client
    readFromSocketIntoBuffer(clientFileDescriptor, messageBuffer);
    

    //send response to client
    //sendDirectoryListing(clientFileDescriptor);
    sendFileContents(clientFileDescriptor, messageBuffer);
    //close connection
    close(clientFileDescriptor);
  }

  //stop server listening
  close(serverSocketFileDescriptor);
}