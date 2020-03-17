#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include "TCPServer.h"
#include "strfuncts.h"

using namespace std;

TCPServer::TCPServer() {}

TCPServer::~TCPServer() {}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port)
{

	struct sockaddr_in servaddr;

	// Set the socket to nonblocking
	_sockfd.setNonBlocking();

	// Load the socket information to prep for binding
	_sockfd.bindFD(ip_addr, port);

}

void TCPServer::connectToCoordinator(const char *ip_addr, unsigned short port)
{
   if (!_sockfd_coord.connectTo(ip_addr, port))
	  throw socket_error("TCP Connection to Coordinator failed!");
}
/*
void TCPServer::listenToCoordinator()
{
	bool connected = true;
	int sin_bufsize = 0;
	ssize_t rsize = 0;

	timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 1000000;

	// Loop while we have a valid connection
	while (connected) {

	  nanosleep(&sleeptime, NULL);
	}
}
*/
/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenToClients()
{

	bool online = true;
	timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 100000000;
	int num_read = 0;

	// Start the server socket listening
	_sockfd.listenFD(5);

	TCPConn *new_conn = new TCPConn();
	std::string msg = "TCPServer is online";
	new_conn->log(msg);

	while (online)
	{
		struct sockaddr_in cliaddr;
		socklen_t len = sizeof(cliaddr);

		if (_sockfd.hasData())
		{
			new_conn = new TCPConn();
			if (!new_conn->accept(_sockfd))
			{
				std::string msg = "Data received on socket but failed to accept.";
				new_conn->log(msg);
				continue;
			}
			std::cout << "***Got a connection***\n";
			fflush(stdout);

			//set the id on this connection
			new_conn->id = std::to_string(uniqueClientId++);
			_connClientList.push_back(std::unique_ptr<TCPConn>(new_conn));

			// Get their IP Address string to use in logging
			std::string ipaddr_str;
			new_conn->getIPAddrStr(ipaddr_str);

			new_conn->sendText("Welcome to the Prime Number Factorization App!\n");
			new_conn->sendMenu();
		}

		// Loop through our client connections, handling them
		std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connClientList.begin();
		while (tptr != _connClientList.end())
		{
			// If the user lost connection
			if (!(*tptr)->isConnected())
			{
				// Log it

				// Remove them from the connect list
				tptr = _connClientList.erase(tptr);
				std::cout << "Connection disconnected.\n";
				fflush(stdout);
				continue;
			}

			std::string cmd = (*tptr)->handleConnection();
			if (cmd.length()> 1) {
				std::string clientId = (*tptr)->id;
				cout << "id = " << clientId << "\n";
				std::string coordRequest = "FACTOR_REQ|" + clientId + "|" + cmd;
				cout << "sending " << coordRequest << "\n";
				_sockfd_coord.writeFD(coordRequest.c_str());
			}
			// Increment our iterator
			tptr++;
		}


		//now handle incoming coordinator messages
		int sin_bufsize = 0;
		ssize_t rsize = 0;

		// If the connection was closed, exit
		if (!_sockfd_coord.isOpen()) {
			cout << "Coordinator socket is closed! So, shutting down...\n";
			shutdown();
			break;
		}

		// Read any response from coord, connect it to a client via the map, then send the response to that client
		std::string buf;
		if (_sockfd_coord.hasData()) {
			if ((rsize = _sockfd_coord.readFD(buf)) == -1) {
				throw std::runtime_error("Read on coordinator socket failed.");
			}

			// Select indicates data, but 0 bytes...usually because it's disconnected
			if (rsize == 0) {
				shutdown();
				break;
			}

			if (rsize > 0) {
				cout << "There is a response from coord...\n";
				std::string response = buf.c_str();
				std::string left, right;
				std::string err = "Error handling factors: Main Server";
				bool bValid = split(response, left, right, '|');
				if (bValid) {
					bValid = split(right, left, right, '|'); //now we have clientId in the left
					if (bValid) {
						// Loop through our client connections to find out which one to send the msg to
						std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connClientList.begin();
						while (tptr != _connClientList.end()) {
							std::string clientId = left;
							cout << "clientId = " << clientId << "\n";
							if ((*tptr)->id == clientId) {
								//if lost connection
								if (!(*tptr)->isConnected())
									std::cout << "This client has disconnected.\n";
								//isolate the factors to send back to the client
								bValid = split(response, left, response, '|');//clean off command
								if (bValid)
									bValid = split(response, left, response, '|');//clean off id
								if (bValid)
									bValid = split(response, left, response, '|');//clean off the original number
								if (bValid) {
									response = "Prime Factors: " + response;
									//send it to client
									(*tptr)->sendText(response.c_str());
									buf.erase(0, response.length());
								} else {
									(*tptr)->sendText(err.c_str());
									buf.erase(0, err.length());
								}
								break;
							}
							// Increment our iterator
							tptr++;
						}
					}
				}
			}
		}

		// So we're not chewing up CPU cycles unnecessarily
		nanosleep(&sleeptime, NULL);
	}

}
/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown()
{

	_sockfd.closeFD();
	_sockfd_coord.closeFD();
}

