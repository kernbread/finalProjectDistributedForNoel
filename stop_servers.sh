# Author: Brian Curran
# Date: 3/17/20
# Description: Run this script to stop the main server and coordinator
#              To execute, run following command:
#              user$ bash start_servers.sh 

# kill main server
for pid in $(ps -ef | grep "./mainserver" | awk '{print $2}'); do kill -9 $pid; done

# kill coordinator
for pid in $(ps -ef | grep "./coordinator" | awk '{print $2}'); do kill -9 $pid; done
