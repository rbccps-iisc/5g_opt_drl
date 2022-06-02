#SUMO
Sumo (Simulation for Urban Mobility) is used to generate traffic models of a realistic urban scenario which can be interfaced 
with vehicular network simulation run in NS3.
Link to install can be found [here](https://sumo.dlr.de/docs/Installing/index.html).

If sumo is built from the source, and installed in $HOME/sumo, the ~/.bashrc file must be edited as follows :
```
export SUMO_HOME=$HOME/sumo
export PATH=$PATH:$SUMO_HOME/bin:$SUMO_HOME/tools
```
Once setup, the *filename*.sumocfg is to be written which will be used in the NS3 simulation code. It requires 2 main files :
1. *filename*.net.xml
2. *filename*.rou.xml

The network file can be generated from a real map using OpenStreetMap. The map data is download as .osm file which can be converted 
to .net.xml file using :
```
netconvert --osm_filename.osm -o map_filename.net.xml

```
These files can be manually written or generated using randomtrips.py provided in the $SUMO_HOME/tools. 
More detailed explaination on using the python script is found
[here](https://towardsdatascience.com/how-to-simulate-traffic-on-urban-networks-using-sumo-a2ef172e564)

The configuration file is as follows:
```
<configuration>
  <input>
    <net-file value="grid.net.xml"/>
    <route-files value="grid.rou.xml"/>
    <additional-files value="rerouter.add.xml"/>
    <random value = "true" />
  </input>
  <time>
    <begin value="0"/>
    <end value="60"/>
  </time>
   <output>
     <fcd-output value="grid.output.xml"/>
   </output>
</configuration>
```
