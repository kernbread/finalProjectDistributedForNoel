/****************************************************************************************
 * tcp_client - connects to a TCP server for two-way comms
 *
 *              **Students should not modify this code! Or at least you can to test your
 *                code, but your code should work with the unmodified version
 *
 ****************************************************************************************/  

#include <stdexcept>
#include <iostream>
#include <getopt.h>
#include "TCPClient.h"
#include "exceptions.h"

using namespace std; 

void displayHelp(const char *execname) {
   std::cout << execname << " -a <ip_addr> -p <port>" << std::endl;
   std::cout <<  "Optionally, add -s to make this a slave node client" << std::endl;
}

// global default values
const unsigned short default_port = 9999;
const char default_IP[] = "127.0.0.1";

int main(int argc, char *argv[]) {

   // Check the command line input
   if (argc < 3) {
      displayHelp(argv[0]);
      exit(0);
   }

   // Read in the IP address from the command line
   std::string ip_addr(default_IP);

   unsigned short port = default_port;
 

   // Get the command line arguments and set params appropriately
   int c = 0;
   long portval;
   bool slave = false;
   while ((c = getopt(argc, argv, "p:a:s")) != -1) {
      switch (c)
      {
      case 'p':
         portval = strtol(optarg, NULL, 10);
         if ((portval < 1) || (portval > 65535)) {
            std::cout << "Invalid port. Value must be between 1 and 65535";
            std::cout << "Format: " << argv[0] << " [<max_range>] [<max_threads>]\n";
            exit(0);
         }
         port = (unsigned short) portval;
         break;
      case 'a':
         ip_addr = optarg;
         break;
      case 's':
         slave = true;
         break;
      default:
         break;
      }
   }

   // Try to set up the server for listening
   TCPClient* client;
   if(slave){
      client = new Slave();
   } else
   {
      client = new TCPClient();
   }
   
   try {
      cout << "Connecting to " << ip_addr << " port " << port << endl;
      client->connectTo(ip_addr.c_str(), port);

   } catch (socket_error &e)
   {
      cerr << "Connection failed: " << e.what() << endl;
      return -1;
   }	   

   cout << "Connection established.\n";

   try {
      client->handleConnection();

      client->closeConn();
      cout << "Client disconnected\n";

   } catch (runtime_error &e) {
      cerr << "Client error received: " << e.what() << endl;
      return -1;      
   }

   return 0;
}
