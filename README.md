# 5g_opt_drl

#### Installation
1. Go to the network folder and run the initial script which downloads ns-3.33, configures and builds. This script needs to be executed only once and it may take a while to finish.
```
$ cd network
$ ./net_init.sh
```
#### RUNNING THE SIMULATION
2. Go to the network folder and run the simulation script
```
$ cd network
$ ./update_scratch.sh
```
```
$ cd network/ns-allinone-3.33/ns-3.33
$ ./waf --run lte_config    
```

#### Integrating ns3-gym with ray\[rllib\]
Please refer the corresponding [README.md](ray-de/examples/ns3-gym-example) at ray-de/examples/ns3-gym-example