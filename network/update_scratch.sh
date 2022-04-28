#!/bin/bash

HOME_DIR=$(pwd)
echo $HOME_DIR
NS3_HOME=$HOME_DIR/ns-allinone-3.33/ns-3.33

#Copy the lte-config code to ns-3 scratch folder
cp -r $HOME_DIR/lte_config $NS3_HOME/scratch/
#cp $HOME_DIR/config.xml $NS3_HOME/
#sudo chmod 777 $NS3_HOME/config.xml

