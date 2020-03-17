# Author: Brian Curran
# Date: 3/17/20
# Description: This script starts NUM_SLAVES slave nodes. To run this script, run the following command:
#              curran$ bash start_slaves.sh

NUM_SLAVES=10

for i in $(eval echo {1..$NUM_SLAVES})
do
	cd slave/src/ && ./slave -s -a 127.0.0.1 -p 9999 >> slave_out.txt 2>&1 &
done
