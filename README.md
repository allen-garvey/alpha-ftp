# Alpha-ftp

Basic ftp client and server

## Dependencies

* gcc 4.8.5 or later
* POSIX compatible operating system 

## Getting Started

* 
* 

## Application Layer Protocol

* Server starts listening for TCP requests on a port
* Client sends message to server in format `CONTROL: -l\n` for directory listing or `CONTROL: -g <filename>\n` for contents of filename
* Server returns message `ERROR: <error_message>\n` if there is an error with the request, or `OK: \n` if command is accepted
* Client starts listening for TCP requests on a port so the data can be transfered. Sends the server a message in the format `TRANSFER: <port_num>\n` with <port_no> being the port the server should use to connect to the client to send the data
* The server connects to the client on the given port and sends the data with the format `DATA: \n<data>\n` where <data> is the requested data. If there was a problem retrieving the data, a message in the format `ERROR: <error_message>\n` will be sent
* The server will then close the data connection and the client will close the control connection

## License

Alpha-ftp is released under the MIT License. See license.txt for more details.