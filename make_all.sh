# Author: Brian Curran
# Date: 3/17/20
# Description: Run this script to configure/make all components of this project. To execute, run the following command:
#              curran$ bash make_all.sh

printf "Building COORDINATOR\n"
cd coordinator && autoreconf -i && ./configure && make && cd ..
printf "\nFinished building COORDINATOR\n"

printf "\nBuilding MAIN_SERVER\n"
cd main_server && autoreconf -i && ./configure && make && cd ..
printf "\nFinished building MAIN_SERVER\n"

printf "\nBuilding SLAVES\n"
cd slave && autoreconf -i && ./configure && make && cd ..
printf "\nFinished building SLAVES\n"
