# Author: Brian Curran
# Date: 3/17/20
# Description: Run this script to make distclean all components of this project. To execute, run the following command:
#              curran$ bash dist_cleanall.sh

printf "\nCleaning COORDINATOR\n"
cd coordinator && make distclean && cd ..
printf "\nFinished cleaning COORDINATOR\n"

printf "\nCleaning MAIN_SERVER\n"
cd main_server && make distclean && cd ..
printf "\nFinished cleaning MAIN_SERVER\n"

printf "\nCleaning SLAVES\n"
cd slave && make distclean && cd ..
printf "\nFinished cleaning SLAVES\n"
