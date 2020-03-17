#include "TCPClient.h"
#include <thread>
#include <ctime>
#include <regex>
#include "exceptions.h"
#include "DivFinderSP.h"
#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/multiprecision/cpp_int.hpp>


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {
	// create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == 0) 
		throw socket_error("Failed to make socket!");

	// BELOW IS FOR TESTING PURPOSES ONLY!	
	// explicitly set client ip address
	//slaveTestMode
	if (true) {
		struct sockaddr_in myAddr;
		myAddr.sin_family = AF_INET;
		myAddr.sin_addr.s_addr = inet_addr("127.0.0.2");
		myAddr.sin_port = 0;
	
		if (bind(sockfd, (struct sockaddr*) &myAddr, sizeof(struct sockaddr_in)) == 0) 
			std::cout << "Bound client" << std::endl;
		else
			std::cout << "Failed to bind client" << std::endl;

	}

	// connect to server
	this->server.sin_addr.s_addr = inet_addr(ip_addr);
	this->server.sin_family = AF_INET;
	this->server.sin_port = htons(port);
	int connection = connect(sockfd, (struct sockaddr*)&this->server, sizeof(this->server));

	if (connection < 0) 
		throw socket_error("Failed to connect to server!");
	
	this->sockfd = sockfd;

	// start receiving thread
	std::thread receivingThread(&TCPClient::receivingThread, this);
	receivingThread.detach(); // make thread a daemon

	// start sending thread
	std::thread sendingThread(&TCPClient::sendingThread, this);
	sendingThread.detach(); // make thread a daemon
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {

	while (!connClosed && !connectionBroke) {
		// check if we got any new messages from the server
		this->mtx1.lock();
		if(!this->receivedMessages.empty()) {
			auto message = this->receivedMessages.front();
			
			if (sanitizeUserInput(message).compare("") != 0) // only display messages that have data
				std::cout << "received: " << message << std::endl;

			this->receivedMessages.pop();
		}
		this->mtx1.unlock();
	}

	// check for broken connection
	if (this->connectionBroke) 
		throw std::runtime_error("Failed to receive heartbeat from server for 8 seconds!");
}

bool TCPClient::sendData(std::string data) {
	if (send(this->sockfd, data.c_str(), data.length(), 0) < 0) {
		std::cout << "Failed to send.\n";
		return false;
	}
	return true;
}

std::string TCPClient::receiveData() {
	char buffer[2048] = "";

	if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
		std::cout << "Failed to receive.\n";
	}

	std::string response(buffer);

	return response;
}

void TCPClient::receivingThread() {
	while (!connClosed && !connectionBroke) {
		auto response = receiveData();

		// check for heartbeat from server
		
		/*
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now(); // get current time
		if (response.compare("HEARTBEAT\n") == 0) {
			this->lastTimeHeartBeatReceived = now;
			continue; // don't push HB's to end user
		} else { // check if it has been > 8 seconds since last hb
			if (now - this->lastTimeHeartBeatReceived > std::chrono::seconds(8))
				this->connectionBroke = true; // signal we have a broken connection
		}
		*/

		this->mtx1.lock();
		this->receivedMessages.push(response);
		this->mtx1.unlock();
	}
}

void TCPClient::sendingThread() {
	while (!connClosed && !connectionBroke) {
		std::string clientMessage;

		//maybe not necessary to lock for the whole sendData() function.
		this->mtx_send.lock();
		if(!this->sendMessages.empty()){
			clientMessage = sendMessages.front();
			auto sending = sendData(clientMessage);
			sendMessages.pop();

		}
		this->mtx_send.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

std::string TCPClient::sanitizeUserInput(const std::string& s) {
	// remove leading/trailing white spaces from user input
	// influence from https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
	std::string leftTrimmed = std::regex_replace(s, std::regex("^\\s+"), std::string(""));
	std::string leftAndRightTrimmed = std::regex_replace(leftTrimmed, std::regex("\\s+$"), std::string(""));

	std::string leftAndRightTrimmedandNoNewLine = std::regex_replace(leftAndRightTrimmed, std::regex("\\n"), std::string(""));
	return leftAndRightTrimmed;
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPClient::closeConn() {
	close(sockfd);
}

void Slave::handleConnection() {

	while (!connClosed && !connectionBroke) {
		// check if we got any new messages from the server
		this->mtx1.lock();
		if(!this->receivedMessages.empty()) {
			auto message = this->receivedMessages.front();
			
			if (sanitizeUserInput(message).compare("") != 0) // only display messages that have data
				std::cout << "received: " << message << std::endl;
			
			Slave::handleMessage(sanitizeUserInput(message));
			this->receivedMessages.pop();
		}
		this->mtx1.unlock();
		if(cancel_op){
			prime_factors.clear();
			cancel_op = false;
			if(slave_div != nullptr){
				slave_div->cancel_op();
				div_thread.join();
				delete slave_div;
				slave_div = nullptr;
			}
		}
		if(!prime_factors.empty()){
/* 			std::cout << "Prime factors are: ";
			for(std::list<LARGEINT>::const_iterator itr = prime_factors.begin(), end = prime_factors.end(); itr != end; itr++) {
				//std::cout << *itr << " ";
				std::cout << "Testing... " + LARGEtostr(*itr) + ",";
			} 
			std::cout << std::endl; */
			std::string pollardResponse = "POLLARD_RESP|" + std::to_string(client_ID) + "|" + std::to_string(slave_ID) + "|" + LARGEtostr(num_to_factor) + "|";
			for(std::list<LARGEINT>::const_iterator itr = prime_factors.begin(), end = prime_factors.end(); itr != end; itr++) {
				pollardResponse = pollardResponse + LARGEtostr(*itr) + ",";
			}
			this->mtx_send.lock();
			sendMessages.push(pollardResponse.substr(0,pollardResponse.size()-1));
			this->mtx_send.unlock();
			prime_factors.clear();
			div_thread.join();
			delete slave_div;
			slave_div = nullptr;
		}
	}
	// check for broken connection
	if (this->connectionBroke) 
		throw std::runtime_error("Failed to receive heartbeat from server for 8 seconds!");
}

void Slave::handleMessage(std::string msg) {
	std::vector<std::string> splitMessage;
	boost::algorithm::split(splitMessage, msg, boost::is_any_of("|"));
 	auto messageType = splitMessage.at(0);
	if(messageType.compare("POLLARD_REQ") == 0) {
		//REQ|SlaveID|ClientID|Number
		client_ID = stoi(splitMessage.at(1));
		slave_ID = stoi(splitMessage.at(2));
		num_to_factor = strtoLARGE(splitMessage.at(3));
		cancel_op = false;
		//run pollards RHO
		slave_div = new DivFinderSP(num_to_factor);
		div_thread = std::thread(&DivFinderSP::PolRho, slave_div, std::ref(prime_factors));

	} else if (messageType.compare("CANCEL_REQ") == 0) {
		//cancel and send CANCEL_RESP to coordinator
		cancel_op = true;
		slave_ID = stoi(splitMessage.at(1));
		mtx_send.lock();
		sendMessages.push("CANCEL_RESP|" + std::to_string(slave_ID));
		mtx_send.unlock();
	}
}

LARGEINT Slave::strtoLARGE(std::string str_num) {
	LARGEINT res = 0;
	for ( size_t i = 0; i < str_num.length(); i++){
		res = res*10 + (str_num.at(i) - '0');
	}
	return res;
}
std::string Slave::LARGEtostr(LARGEINT i) {
	std::stringstream ss;
	ss << i;
	return ss.str();
}