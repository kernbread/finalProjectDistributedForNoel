#include <stdexcept>
#include <strings.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include "TCPConn.h"
#include "strfuncts.h"
#include <time.h>
#include <ctime>

const std::string logfilename = "server.log";

TCPConn::TCPConn()
{
	// LogMgr &server_log):_server_log(server_log)
	//std::srand(std::time(nullptr)); // use current time as seed for random generator
	//id = std::rand();
}

TCPConn::~TCPConn()
{

}

void TCPConn::log(std::string &msg)
{
	time_t my_time = time(NULL);
	msg += " ";
	msg += ctime(&my_time);

	FileFD logger(logfilename.c_str());
	if (!logger.openFile(FileFD::appendfd))
		throw pwfile_error("Could not open log file to write msg");

	std::vector<char> msg_vector = std::vector<char>(msg.begin(), msg.end());

	try
	{
		logger.writeBytes(msg_vector);
	} catch (logfile_error &px)
	{
		std::cout << "logger write error";
		fflush(stdout);
	}
	logger.closeFD();
}

/**********************************************************************************************
 * accept - simply calls the acceptFD FileDesc method to accept a connection on a server socket.
 *
 *    Params: server - an open/bound server file descriptor with an available connection
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPConn::accept(SocketFD &server)
{
	return _connfd.acceptFD(server);
}

/**********************************************************************************************
 * sendText - simply calls the sendText FileDesc method to send a string to this FD
 *
 *    Params:  msg - the string to be sent
 *             size - if we know how much data we should expect to send, this should be populated
 *
 *    Throws: runtime_error for unrecoverable errors
 **********************************************************************************************/

int TCPConn::sendText(const char *msg)
{
	return sendText(msg, strlen(msg));
}

int TCPConn::sendText(const char *msg, int size)
{
	if (_connfd.writeFD(msg, size) < 0)
	{
		return -1;
	}
	return 0;
}

/**********************************************************************************************
 * handleConnection - performs a check of the connection, looking for data on the socket and
 *                    handling it based on the _status, or stage, of the connection
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

std::string TCPConn::handleConnection()
{

	timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 100000000;

	std::string cmd = "";

	try
	{
		switch (_status)
		{

			case s_menu:
				cmd = getMenuChoice();
				break;

			default:
				throw std::runtime_error("Invalid connection status!");
				break;
		}
	} catch (socket_error &e)
	{
		std::cout << "Socket error, disconnecting.";
		disconnect();
		return "";
	}

	nanosleep(&sleeptime, NULL);
	return cmd;
}

/**********************************************************************************************
 * getUserInput - Gets user data and includes a buffer to look for a carriage return before it is
 *                considered a complete user input. Performs some post-processing on it, removing
 *                the newlines
 *
 *    Params: cmd - the buffer to store commands - contents left alone if no command found
 *
 *    Returns: true if a carriage return was found and cmd was populated, false otherwise.
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

bool TCPConn::getUserInput(std::string &cmd)
{
	std::string readbuf;

	// read the data on the socket
	_connfd.readFD(readbuf);

	// concat the data onto anything we've read before
	_inputbuf += readbuf;

	// If it doesn't have a carriage return, then it's not a command
	int crpos;
	if ((crpos = _inputbuf.find("\n")) == std::string::npos)
		return false;

	cmd = _inputbuf.substr(0, crpos);
	_inputbuf.erase(0, crpos + 1);

	// Remove \r if it is there
	clrNewlines(cmd);

	return true;
}

/**********************************************************************************************
 * getMenuChoice - Gets the user's command and interprets it, calling the appropriate function
 *                 if required.
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

std::string TCPConn::getMenuChoice()
{
	if (!_connfd.hasData())
		return "";
	std::string cmd;
	if (!getUserInput(cmd))
		return "";
	lower(cmd);

	std::string ip;
	getIPAddrStr(ip);

	std::string msg;

	if (cmd.compare("exit") == 0)
	{
		std::string msg = "Session disconnect from IP: " + ip + " - session terminated\n";
		log(msg);
		_connfd.writeFD("Disconnecting...goodbye!\n");
		disconnect();
	}
	else if (cmd.compare("menu") == 0)
	{
		sendMenu();
	}
	else
	{
		if (isNum(cmd))
			msg = cmd;
		else
		{
			msg = "Unrecognized command: ";
			msg += cmd;
			msg += "\n";
			_connfd.writeFD(msg);
			msg = "";
		}
	}
	return msg;
}

bool TCPConn::isNum(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

/**********************************************************************************************
 * sendMenu - sends the menu to the user via their socket
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::sendMenu()
{
	std::string menustr;
	menustr += "Available choices: \n";
	//menustr += "  1). Find prime factors of 8051: returns 83*97\n";
	//menustr += "  2). A larger number: 9483746374657, returns: 101*1187\n";
	//menustr += "  3). Getting larger: 83748574838495849483, returns: 37*89*137*185637250523663\n";
	//menustr += "  4). 20 digits: 39483928374837104097, returns: 3*3*3*19*27967*29129*94478183\n";
	//menustr += "  5). 25 digits: 3829387463527123647384769, returns: 29*41*15413*2798639*74664368503\n";
	menustr += "  Enter a number to be factored: \n";
	menustr += "  Menu - display this menu\n";
	menustr += "  Exit - disconnect.\n";

	_connfd.writeFD(menustr);
}

/**********************************************************************************************
 * disconnect - cleans up the socket as required and closes the FD
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::disconnect()
{
	_connfd.closeFD();
	_connfdcoord.closeFD();
}

/**********************************************************************************************
 * isConnected - performs a simple check on the socket to see if it is still open 
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
bool TCPConn::isConnected()
{
	return _connfd.isOpen();
}

/**********************************************************************************************
 * getIPAddrStr - gets a string format of the IP address and loads it in buf
 *
 **********************************************************************************************/
void TCPConn::getIPAddrStr(std::string &buf)
{
	return _connfd.getIPAddrStr(buf);
}

