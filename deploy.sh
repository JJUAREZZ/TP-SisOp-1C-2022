#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nBuilding projects...\n\n"
make -C ./consola/consola
make -C ./kernel/kernel
make -C ./memoria/memoria
make -C ./cpu/cpu
echo -e "\n\nDeploy done!\n\n"