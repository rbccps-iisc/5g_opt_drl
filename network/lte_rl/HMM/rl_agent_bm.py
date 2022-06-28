#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from ns3gym import ns3env
import pandas as pd
import numpy as np


import sumolib
import mchmm._mc as mc
import mchmm._hmm as hmm
import os,sys
import sysv_ipc

# Loading the Map

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:   
    sys.exit("please declare environment variable 'SUMO_HOME'")

net = sumolib.net.readNet('grid.net.xml')

for route in sumolib.xml.parse_fast("grid.rou.xml", 'route', ['edges']):
    edge_ids = route.edges.split()

print(edge_ids)

N= 3 # Number of Hidden States
T= 20# Length of Observed Sequence
sts_seq=[]
for item in edge_ids:
    sts_seq.append(np.random.randint(N,size=1)[0])
obs_space= edge_ids

"""
obs_seq = edge_ids
sts_seq=[0,1,1,2,1,1,1,2,0,1,1,2,0,1]
init_seq=['B2B1', 'B0A0', 'A0A1', 'A1B1', 'B1C1', 'B2A2', 'A2A1', 'A1B1', 'B0A0', 'A0A1', 'A1B1', 'B0A0', 'A0A1', 'C1C0']
theta = hmm.HiddenMarkovModel().from_seq(obs_seq,sts_seq)
tp = pd.DataFrame(theta.tp, index=theta.states, columns=theta.states, dtype=float)
ep = pd.DataFrame(theta.ep, index=theta.states, columns=theta.observations, dtype=float)
"""

parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=1,
                    help='Number of iterations, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
iterationNum = int(args.iterations)

port = 5555
simTime = 210  # seconds
stepTime = 3  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--testArg": 123}
debug = False

enb_loc=[100,100,0]

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)

env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)


currIt = 0
stepIdx = 0
labels =[]
prev_val="laneID"

f=open('rewards.txt','a')
try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
    
        print("Step: ", stepIdx)
        print("---obs:", obs)

        new_obs =[]
        rewardsum=0

        while True:
            stepIdx += 1
            action = env.action_space.sample()
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)

            # Create shared memory object
            memory = sysv_ipc.SharedMemory(123456)

            # Read value from shared memory
            memory_value = memory.read()

            # Find the 'end' of the string and strip
            i = memory_value.find(b'\0')
            if i != -1:
                memory_value = memory_value[:i]

            lane = memory_value.decode("utf-8")


            if lane != prev_val and  lane[0] != ':':  # if lane changed and it's not a junction

                if lane in edge_ids :
                    new_obs.append(lane)
                    prev_val = lane
                    if currIt !=0:
                        p = [theta.pi[i]*ep.loc[i].at[lane] for i in range(N)]

                        curr_state = np.argmax(p)
                        idx_state = tp.idxmax(axis=1)
                        next_state=idx_state[curr_state]
                     
                        idx_link = ep.idxmax(axis='columns')
                        next_link=idx_link[next_state]
                       
                        print("\n" +next_link+ "\n")

                        if next_link[1][0] != ':' : # for junctions
                            laneShape= net.getEdge(next_link).getShape()

                        lane_end = laneShape[1]
                        x,y = lane_end
                        d = ((enb_loc[0] - x)**2 + (enb_loc[1] - y)**2)**(0.5) 

                        obs.append(d)

            rewardsum+=reward

            print("---obs, reward, done, info: ", obs, reward, done)

            if done:
                f.write(str(rewardsum/stepIdx))
                f.write("\n")
                stepIdx = 0
                if currIt + 1 < iterationNum:
                    env.reset()
                break
        
        new_obs=new_obs[:T]
        print(new_obs)
        if currIt ==0:

            theta = hmm.HiddenMarkovModel().from_baum_welch(new_obs, obs=obs_space, states=[0,1,2],pi=[])
            tp = pd.DataFrame(theta.tp, index=theta.states, columns=theta.states, dtype=float)
            ep = pd.DataFrame(theta.ep, index=theta.states, columns=theta.observations, dtype=float)

        else:

            tp = pd.DataFrame(theta.tp, index=theta.states, columns=theta.states, dtype=float)
            ep = pd.DataFrame(theta.ep, index=theta.states, columns=theta.observations, dtype=float)
            t=tp.to_numpy(copy=True)
            e=ep.to_numpy(copy=True)
            pi=(theta.pi).tolist()

            tp.to_csv("transition_prob.csv", sep='\t', encoding='utf-8')
            ep.to_csv("emission_prob.csv", sep='\t', encoding='utf-8')
            theta = hmm.HiddenMarkovModel().from_baum_welch(new_obs, obs=obs_space, states=[0,1,2],tp=t,ep=e,pi=pi)

        currIt += 1
        if currIt == iterationNum:
        	break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    f.close()
    env.close()
    print("Done")
