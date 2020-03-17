#ifndef TCPCONN_H
#define TCPCONN_H

#include "FileDesc.h"

const int max_attempts = 2;

// Methods and attributes to manage a network connection, including tracking the username
// and a buffer for user input. Status tracks what "phase" of login the user is currently in
class TCPConn 
{
public:
   TCPConn(); /* LogMgr &server_log*/

   ~TCPConn();

   bool accept(SocketFD &server);

   int sendText(const char *msg);
   int sendText(const char *msg, int size);

   std::string handleConnection();
   void sendMenu();
   std::string getMenuChoice();
   bool isNum(const std::string& s);

   
   bool getUserInput(std::string &cmd);

   void disconnect();
   bool isConnected();

   unsigned long getIPAddr() { return _connfd.getIPAddr(); };
   void getIPAddrStr(std::string &buf);

   //jb add
   std::string id;
   void log(std::string &msg);

private:


   enum statustype { s_menu };

   statustype _status = s_menu;

   SocketFD _connfd;
   SocketFD _connfdcoord;
 
   std::string _inputbuf;
   std::string _inputbufcoord;



};


#endif
