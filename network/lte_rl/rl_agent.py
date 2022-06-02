import argparse
from ns3gym import ns3env
import pandas as pd
import numpy as np


import sumolib
import os,sys

# Loading the Map--------------------------------------------------------------

import os, sys
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:   
    sys.exit("please declare environment variable 'SUMO_HOME'")

net = sumolib.net.readNet('grid.net.xml')
#------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=0,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=1,
                    help='Number of iterations, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
iterationNum = int(args.iterations)

port = 5555
simTime = 60 # seconds
stepTime = 2  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--testArg": 123}
debug = False

# Need to be same as defined in NS3 simulation
enb_loc=[[600,400,0],[900,400,0]]

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
# simpler:
#env = ns3env.Ns3Env()
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)


currIt = 0
stepIdx = 0
labels =[]

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
    
        print("Step: ", stepIdx)
        print("---obs:", obs)
	
        while True:
            stepIdx += 1
            action = env.action_space.sample()
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)
            
            # The last element of observation vector is lane ID
            # Extract it and build the state transition matrix across iterations
            # Use the matrix to predict next possible lane -- > argmax P(sj/si) 
            # Once next lane is predicted, calculate future distance to Base Stations and use it as the observation parameter
            
            # Hence, this section of code converts the last elements of Observation Vector( lane ID )  received from NS3 to usable parameter as described above
            
            #--------------------------------------------------------------------------------------------------------------------------------------
  
            # Initital Condition
            
            if stepIdx == 1 and currIt == 0 :# DataFrame created at the first iteration
            	curr_lane = chr(int(obs[3]))+ chr(int(obs[4])) + chr(int(obs[5])) + chr(int(obs[6])) + chr(int(obs[7])) + chr(int(obs[8]))
            	
            	df = pd.DataFrame([(0)],columns=[curr_lane],index=[curr_lane])
            	labels.append(curr_lane)
            	
            else :
            	
            	prev_lane = curr_lane
            	curr_lane = chr(int(obs[3]))+ chr(int(obs[4])) + chr(int(obs[5])) + chr(int(obs[6])) + chr(int(obs[7])) + chr(int(obs[8]))
            	   	
            	if curr_lane not in labels:
            		labels.append(curr_lane)
            		df.loc[curr_lane]=0
            		df[curr_lane]=0
            		if stepIdx != 1:# Arrangement for Second Iteration and Onward
            			df[prev_lane][curr_lane] = 1
            	else:
            		if prev_lane != curr_lane:
            			df[prev_lane][curr_lane] += 1
            	labels.append(curr_lane)  
            #--------------------------------------------------------------------------------------------------------------------------------------
            
            # Get next predicted lane junction coordinates
            max_id = np.argmax(df.loc[curr_lane])
            parameter = df.columns[max_id]
            
            #next_junction =  net.getEdge(parameter).getToNode().getID()
            if parameter[0] != ':' : # Lane ID begins with ':' for junctions
            	laneShape= net.getLane(parameter).getShape()
            	
            #Get Attached BS Location
            cell_id = obs[2]
            pos =  enb_loc[int(cell_id) -1]
            
            
            lane_end = laneShape[1]
            x,y = lane_end
            pred_dist = ((pos[0] - x)**2 + (pos[1] - y)**2)**(0.5)
            
            obs[2] = pred_dist
            obs=obs[0:3]
            
            print("---obs, reward, done, info: ", obs, reward, done, info)

            if done:
                stepIdx = 0
                if currIt + 1 < iterationNum:
                    env.reset()
                break

        currIt += 1
        if currIt == iterationNum:
        	print(df)
        	break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")
