# Large Number Factorization Distributed System
## By: Brian Curran, Matt Eilertson, and John Beighle

### This directory contains the following components:
	1. Coordinator: job scheduler and communication medium between main server and slave nodes.
		- by default, runs on 127.0.0.1:9999
	2. Main Server: server that handles all client connections to factoring system.
		- by default, runs on 127.0.0.1:5050
	3. TCP Client: a client who connects to the main server to request numbers to factorize.
	4. Slave(s): a tcpclient that connects to the coordinator and factorizes numbers passed to it.
		- by default, runs on 127.0.0.2

### Building/Unbuilding:
	To build this project, simply run the following command:
		curran$ bash make_all.sh
	-	- NOTE: edit coordinator/include/TCPServer.h if you wish and modify numberOfJobsPerClientReq to whatever you want.
		- NOTE2: if you modify after building, you must run the following command:
			curran$ cd coordinator && make && cd ..

	To unbuild this project, run the following command:
		curran$ bash dist_cleanall.sh 

### Running:
	To start the servers (coordinator and main server), run the following command:
		curran$ bash start_servers.sh

	To start the slave nodes, run the following command:
		curran$ bash start_slaves.sh

		- NOTE: edit start_slaves.sh if you wish and modify NUM_SLAVES if you wish to start more/less slave nodes.

	To start running a client to connect and factorize numbers, run the following command:
		curran$ ./main_server/src/tcpclient 127.0.0.1 5050

	<b>Ensure that you run start_servers.sh before starting slave nodes (otherwise, slave nodes won't be able to connect to coordinator)</b>
	
### Stopping:
	To stop the servers, run the following command:
		curran$ bash stop_servers.sh

	To stop the slave nodes, run the following command:
		curran$ bash stop_slaves.sh

### Ensuring Processes Started/Stopped:
	To ensure a process started/stopped, you can run the following command:
		curran$ ps aux | grep <insert_process_name> 
		e.g. curran$ ps aux | grep coordinator

### Debugging:
	Coordinator: logs to coordinator/src/server.log
	Main Server: logs stdout to main_server/src/main_server_out.txt and to log file main_server/src/server.log 
 	TCP Client: logs to stdout 
	Slaves: logs stdout to slave/src/slave_out.txt 
		- warning: not properly logging for slaves, so this file will be of limited usefulness...  

 
