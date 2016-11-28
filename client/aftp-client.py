#!/usr/bin/env python

import sys
from socket import *
import re

#alpha ftp client
#by: Allen Garvey


#############################################################
# Parse server response functions
#############################################################
#returns true if message is server error message
#matching based on: http://stackoverflow.com/questions/180986/what-is-the-difference-between-pythons-re-search-and-re-match
def isErrorMessage(message):
	if re.match("^ERROR: ", message):
		return True
	return False

#returns error message by extracting error header
#substitution based on: http://stackoverflow.com/questions/16720541/python-string-replace-regular-expression
def extractErrorMessage(errorMessage):
	return re.sub("^ERROR: ", "", errorMessage)

#parses server response to command
#if successful will return int of lines of data to be sent
#if error will print error and exit
def parseServerCommandResponse(serverResponse):
	#check for error
	
	if isErrorMessage(serverResponse):
		print extractErrorMessage(serverResponse)
		sys.exit(1)
	#if we're here, must be successful, so parse lines
	rawLines = re.sub("^OK: ", "", serverResponse)
	#convert to integer
	linesOfData = int(rawLines)
	return linesOfData

#############################################################
# Setup TCP connection functions
#############################################################

#connects to server at hostName and portNum using TCP
#returns socket object
#based on socket programming slides
def connectToServer(hostName, portNum):
	clientSocket = socket(AF_INET, SOCK_STREAM)
	try:
		clientSocket.connect((hostName, portNum))
	except Exception:
		print "Could not connect to " + hostName + ":" + str(portNum)
		sys.exit(1)
	return clientSocket

#creates binds to port specified by portNum so that it can listen
#for TCP connections
#returns socket object or false if it fails
#based on socket programming slides
def createDataConnection(portNum):
	serverDataSocket = socket(AF_INET, SOCK_STREAM)
	try:
		serverDataSocket.bind(("", portNum))
	except Exception:
		return False
	return serverDataSocket


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
	#CONSTANTS
	MESSAGE_LENGTH = 1024

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

	#format command for server
	#should be in format "CONTROL: <command_name> [file_name]\n"
	if commandName == "-l":
		formattedCommand = "CONTROL: -l\n"
	else:
		#no need to check commandArguments length first since we already validated this
		formattedCommand = "CONTROL: -g " + commandArguments[5] + "\n"

	#send command to server
	controlConnection.send(formattedCommand)

	#receive number of lines of data or error
	serverResponse = controlConnection.recv(MESSAGE_LENGTH)
	#will return int of lines of data to be received
	#will exit with error if server returns error message
	linesOfData = parseServerCommandResponse(serverResponse)

	#start data connection
	dataConnectionSocket = createDataConnection(dataPortNum)
	#check that data connection succeeded
	if dataConnectionSocket == False:
		print "Could not bind to port " + str(portNum) + " for data connection"
		#close control connection, since server will not do that
		controlConnection.close()
		sys.exit(1)


	#format message to send data port number to server
	dataPortNumMessage = "TRANSFER: " + str(dataPortNum) + "\n"
	linesOfDataReceived = 0
	print "Lines of data is: " + str(linesOfData)
	
	#listen for data or error and print it out to user
	#based on socket programming slides
	dataConnectionSocket.listen(1)
	#send message to server with data port number
	controlConnection.send(dataPortNumMessage)
	clientConnectionSocket, clientAddress = dataConnectionSocket.accept()
	while linesOfDataReceived < linesOfData:
		line = clientConnectionSocket.recv(MESSAGE_LENGTH)
		#check for error - if there is one print error message and stop listening
		#server should close data connection
		if isErrorMessage(line):
			print extractErrorMessage(line)
			break
		#print data
		else:
			#count newlines so we know how many lines of data received
			#based on: http://stackoverflow.com/questions/1155617/count-occurrence-of-a-character-in-a-string
			linesReceived = line.count("\n")
			#end of file won't have newline
			if linesReceived == 0:
				linesReceived = 1
			print "printing line " + str(linesOfDataReceived) + " of length " + str(len(line))
			#print without newline
			#based on: http://stackoverflow.com/questions/493386/how-to-print-in-python-without-newline-or-space
			sys.stdout.write(line)
			sys.stdout.flush()
			linesOfDataReceived += linesReceived


	#close control connection (server will close data connection)
	controlConnection.close()


