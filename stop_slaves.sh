# Author: Brian Curran
# Date: 3/17/20
# Description: This script stops all running slave nodes. To execute, run the following command:
#              curran$ bash stop_slaves.sh

for pid in $(ps -ef | grep "./slave" | awk '{print $2}'); do kill -9 $pid; done
