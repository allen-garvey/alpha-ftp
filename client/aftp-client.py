#!/usr/bin/env python

import sys
from socket import *

#alpha ftp client
#by: Allen Garvey


#############################################################
# Setup control connection functions
#############################################################

#connects to server at hostName and portNum using TCP
#returns connection or false if can't connect
#based on socket programming slides
def connectToServer(hostName, portNum):
	clientSocket = socket(AF_INET, SOCK_STREAM)
	try:
		clientSocket.connect((hostName, portNum))
	except Exception:
		return False
	return clientSocket

#############################################################
# Command line arguments validation functions
#############################################################
def printUsage(programName):
	print "usage: python " + programName + " <server_host> <server_port> <data_connection_port> <-l | -g> [file_name]"


#returns true if port num is numeric and in valid port number range
def isValidPortNum(portNumString):
	#checking if is numeric based on:
	#http://stackoverflow.com/questions/1265665/python-check-if-a-string-represents-an-int-without-using-try-except
	try: 
		portNum = int(portNumString)
	except ValueError:
		return False
	if portNum > 0 and portNum <= 65535:
		return True
	return False


#validate command arguments are valid and in correct format:
# program_name server_host server_port data_connection_port <-l or -g> [file_name if command is -g]
def validateCommandArguments(commandArguments):
	commandArgumentsLength = len(commandArguments)
	#check to make sure command line arguments is a valid length
	if commandArgumentsLength != 5 and commandArgumentsLength != 6:
		printUsage(commandArguments[0])
		sys.exit(1)
	
	commandName = commandArguments[4]
	#check that commandName is either "-g" or "-l"
	if commandName != "-g" and commandName != "-l":
		printUsage(commandArguments[0])
		sys.exit(1)

	#check for number of arguments for -l command
	if commandName == "-l" and commandArgumentsLength != 5:
		printUsage(commandArguments[0])
		sys.exit(1)

	#check for file name if command is '-g'
	if commandName == "-g" and commandArgumentsLength != 6:
		print "file name is required with -g command"
		sys.exit(1)
	#check that port numbers are valid
	serverPortString = commandArguments[2]
	dataPortString = commandArguments[3]
	if isValidPortNum(serverPortString) == False:
		print serverPortString + " is not a valid port number"
		sys.exit(1)
	if isValidPortNum(dataPortString) == False:
		print dataPortString + " is not a valid port number"
		sys.exit(1)
	#check that server port and data port or not the save
	if serverPortString == dataPortString:
		print "Server port and data port numbers must be different"
		sys.exit(1)



#############################################################
# Main function
#############################################################
if __name__ == '__main__':
	#check for command line arguments
	commandArguments = sys.argv
	validateCommandArguments(commandArguments)
	#if we are here, command arguments are valid
	serverHostName = commandArguments[1]
	#convert port numbers to int
	#no need to use try/catch, since we did that in validation so we know they are numeric
	serverPortNum = int(commandArguments[2])
	dataPortNum = int(commandArguments[3])
	commandName = commandArguments[4]

	#create control connection to server
	controlConnection = connectToServer(serverHostName, serverPortNum)
	#check that connection succeeded
	if controlConnection == False:
		print "Could not connect to " + serverHostName + ":" + str(serverPortNum)
		sys.exit(1)

	#send command to server


	#receive number of lines of data or error

	#start data connection

	#send data connection port to server

	#listen for data or error and print it out to user


	#close control connection (server will close data connection)



