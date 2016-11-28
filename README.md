# Alpha-ftp

Basic ftp client and server

## Dependencies

* gcc 4.8.5 or later
* POSIX compatible operating system 

## Notes

Throughout this README, use of backticks (`) should not be typed, they are used to signify the start and end of commands

## Getting Started

* Upload this project to flip
* `cd` into the project directory (the directory containing this README)
* Type `make` to compile the server
* Start the server by typing `./server/aftp-server <port_num>`, where <port_num> is the port number you would like the server to listen on
* Start the client by typing `python ./client/aftp-client.py <host_name> <server_port> <data_port> <command> [file_name]` where <host_name> is the server hostname (returned by running the `hostname` command on the server), <server_port> is the port the server is listening on, <data_port> is the port you would like the client to open the data connection on, <command> is either `-l` or `-g` and [file_name] for file to retrieve if the command is `-g` 

## Application Layer Protocol

* Server starts listening for TCP requests on a port

* Client sends message to server in format `CONTROL: -l\n` for directory listing or `CONTROL: -g <filename>\n` for contents of filename

* Server returns message `ERROR: <error_message>\n` if there is an error with the request, or `OK: <line_count>\n` if command is accepted, with line_count being the number of lines of data that will be sent

* If the server doesn't send back an error message, client starts listening for TCP requests on a port so the data can be transfered. Sends the server a message in the format `TRANSFER: <port_num>\n` with <port_no> being the port the server should use to connect to the client to send the data.

* The server connects to the client on the given port and sends the data line by line (up to the given number of lines indicated in the OK message) in the format `<data>` where <data> is the requested data. If there was a problem retrieving the data, a message in the format `ERROR: <error_message>\n` will be sent instead

* The server will then close the data connection, once it has finished transmitting, and the client will close the control connection once it has received all the data from the server

## License

Alpha-ftp is released under the MIT License. See license.txt for more details.