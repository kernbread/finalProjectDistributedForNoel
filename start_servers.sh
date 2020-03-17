# Author: Brian Curran
# Date: 3/17/20
# Description: Run this script to start the main server and coordinator
#              To execute, run following command:
#              user$ bash start_servers.sh 

# start coordinator
cd coordinator/src && ./coordinator > coord_out.txt 2>&1 &

sleep 2

# start main server
cd main_server/src && ./mainserver > main_server_out.txt 2>&1 &

