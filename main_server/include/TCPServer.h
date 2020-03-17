#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <list>
#include <memory>
#include <map>
#include "Server.h"
#include "FileDesc.h"
#include "TCPConn.h"

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void connectToCoordinator(const char *ip_addr, unsigned short port);
   void listenToClients();
   //void listenToCoordinator();
   void shutdown();


private:
   // Class to manage the server socket for client connections
   SocketFD _sockfd;
   // Class to manage the server socket connecting to the coordinator
   SocketFD _sockfd_coord;
   int uniqueClientId = 0;

   // List of TCPConn objects to manage connections
   std::list<std::unique_ptr<TCPConn>> _connClientList;
   std::unique_ptr<TCPConn> _connCoord;
   //std::map<int, std::unique_ptr<TCPConn>> clientMap;
};
#endif
