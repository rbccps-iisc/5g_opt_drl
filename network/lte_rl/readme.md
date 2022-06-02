### LTE RL with SUMO Interface

NS3Gym is used to implement RL by interfacing NS3 simulation with RL agent.
Communication with SUMO is done using TraCI (Transport Controller Interface).

TraCI installation is found [here](https://github.com/vodafone-chair/ns3-sumo-coupling).

NS3Gym requires following function to be implemented:

1. #### Ptr<OpenGymSpace> GetObservationSpace();
  
  The space is box type with shape = number of observation parameters
  
2. #### Ptr<OpenGymSpace> GetActionSpace();
  
  The Action Space is Discrete, that is it outputs values from 0 to N-1. 
  As of the now, these values are mapped to 4 power levels of the serving BS to modulate it.
  
3. #### Ptr<OpenGymDataContainer> GetObservation();
  
  The observation Vector is [RP1, RP2 , CellID, LaneID]
  
4. #### float GetReward();
  
  Reward is chosen to be throughput at the receiver. This is calculated every envStep time interval.

5. #### bool ExecuteActions(Ptr<OpenGymDataContainer> action);
  
  In this function, the Transmit Powers of the BS are set according to the action.

The agent takes action, and collects **observation** and **reward** after every **envStep** time. In NS3, 
Simulator::Scheduler() is also called every **envStep** time to simulate the same time interval. 

### RL Agent
The received Observtion vector element LaneId is used to construct the Transition Matrix which is reflective of User Driving Behaviour.
Using the matrix, the the predicted future distance to the serving BS is calculated, and this inturn is used as observation.
Hence, now the final Observation Vector is converted from [RP1, RP2, CellID, LaneID] to [RP1, RP2, future_distance]. Each iteration,
different path is chosen for training to be meaningful.
  
The program can be run as follows :
```
  ./rl_agent.py --start=1 --iterations=100
```
Or using two terminals :
  
NS3 terminal :
```
  ./waf --run sumo_rl.cc
```
RL terminal
```
  ./rl_agent.py --start=0 --iterations=100
```
 
