# Installation prerequisites for the network simulation NS3 version 3.33

#minimal requirements for Python API users (release 3.30 and newer, and ns-3-dev)

sudo apt install g++ python3 python3-dev pkg-config sqlite3 cmake -y
sudo apt install python3-setuptools git -y
#For Ubuntu 20.10 and earlier, the single 'qt5-default' package suffices  for Netanim animator
sudo apt install qt5-default -y
#Support for ns-3-pyviz visualizer
sudo apt install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3 -y
#Support for MPI-based distributed emulation
sudo apt install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev -y
#Support for bake build tool
sudo apt install autoconf cvs bzr unrar -y
#debugging
sudo apt install gdb valgrind -y
#Support for utils/check-style.py code style check program
sudo apt install uncrustify -y
#Doxygen and related inline documentation
sudo apt install doxygen graphviz imagemagick -y
sudo apt install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk -y
sudo apt install python3-sphinx dia -y
#GNU Scientific Library (GSL) support for more accurate 802.11b WiFi error models
sudo apt install gsl-bin libgsl-dev libgslcblas0 -y

#To read pcap packet traces
sudo apt install tcpdump -y
sudo apt install wireshark -y

sudo apt install sqlite sqlite3 libsqlite3-dev -y
sudo apt install libxml2 libxml2-dev -y
sudo apt install cmake libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
python3 -m pip install --user cxxfilt
sudo apt install libgtk-3-dev

sudo apt install vtun lxc uml-utilities -y
sudo apt install libxml2 libxml2-dev libboost-all-dev

#install libzmq libraries
sudo apt-get install libzmq5 libzmq3-dev libczmq-dev libczmq4 libxml2 libxml2-dev


#Downloading ns-3 Using a Tarball
wget https://www.nsnam.org/releases/ns-allinone-3.33.tar.bz2
tar -xvf ns-allinone-3.33.tar.bz2

HOME_DIR=$(pwd)
echo $HOME_DIR
NS3_HOME=$HOME_DIR/ns-allinone-3.33/ns-3.33
#PATCH_PATH=$HOME_DIR/patches

#cd $NS3_HOME
ls -lrt
cd $NS3_HOME
./waf configure --enable-examples --enable-tests
./waf
